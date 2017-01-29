/*
 * MultiThrdSerial.cpp
 *
 *  Created on: 2016-4-17
 *      Author: lcz
 */

#include <termios.h>            /* tcgetattr, tcsetattr */
#include <stdio.h>              /* perror, printf, puts, fprintf, fputs */
#include <sys/types.h>
#include <unistd.h>             /* read, write, close */
#include <fcntl.h>              /* open */
#include <string.h>             /* bzero */
#include <math.h>
#include <myComm.h>
#include <pthread.h>

#include "AppConfig.h"
//#include "DevComm.h"
#include "MultiThrdSerial.h"

//pthread_mutex_t can_writeLock = PTHREAD_MUTEX_INITIALIZER;

void * ComProc(void * param) {
	SET_THRD_NAME();
	MultiThrdSerial* pComm = (MultiThrdSerial*) param;
	fd_set rd;
	if (pComm->directConn() || pComm->connBy485()) {
		FD_ZERO(&rd);
		FD_SET(pComm->m_FD, &rd);
	}
	unsigned char _buf[256];
	int nread;
	while (pComm->isRunning()) {
//		if (!(pComm->isOnRcv())) {
//			usleep(2000);
//			continue;
//		}
		if (pComm->directConn() || pComm->connBy485()) {
			FD_ZERO(&rd);
			FD_SET(pComm->m_FD, &rd);
			while (FD_ISSET(pComm->m_FD, &rd)) {
				if (select(pComm->m_FD + 1, &rd, NULL, NULL, NULL) < 0)
					perror("select error!\n");
				else
					while((nread = pComm->readSerial(_buf, 256)) > 0) ;
			}
		} else {// CAN Bus
			while ((nread = pComm->readSerial(_buf, 256)) > 0) ;
		}
	}
	return ((void *) 0);
}
MultiThrdSerial::MultiThrdSerial() {
}
void MultiThrdSerial::clearBuf() {
	if (m_portBuf) {
		delete m_portBuf;
		m_portBuf = NULL;
	}
}
MultiThrdSerial::~MultiThrdSerial() {
	closePort();
	clearBuf();
}
void MultiThrdSerial::putDat(unsigned char * c, unsigned int len) {
	unsigned int sz = len;
	mtx_inDat.lock();
#if DEBUG_COMM_IO
	unsigned was = m_pos;
#endif
//	cout << "[put +++ ";
	if ((m_pos + len) > m_MaxSize)
		sz = m_MaxSize - m_pos;
	for (unsigned int i = 0; i < sz; ++i)
		m_portBuf[m_pos + i] = c[i];
	m_pos += sz;
//	cout << "pos=" << m_pos << "]\n";
#if DEBUG_COMM_IO
		SHOW_TIME("read:");
		printf("thread(%ld):", (long int)syscall(224));
		cout << "read (not CAN) <= FD=" << m_FD << "|old pos=" << was << "|len=" << len <<"\t";
		int k;
		for (k = 0; k < m_pos; ++k)
			printf("%02X.", m_buf[k]);
		printf("\n");
#endif

//	cout << "[put --- ]" << endl;
	mtx_inDat.unlock();
}

u32 MultiThrdSerial::writeSerial(unsigned char * pBuf, u32 dwLength) /*些串口数据*/
{
	int rtn;
#if DEBUG_COMM_IO
	SHOW_TIME("wrt ");
	printf("thread(%ld)|", (long int)syscall(224));
	cout << "write => FD=" << m_FD << ":\t\n";
#endif
#if DEBUG_COMPORT_HEX_DAT
	if (dwLength < 18) {
		printf("\t\t ---->");
		unsigned int k;
		for (k = 0; k < dwLength; ++k)
			printf(" %02x.", *(pBuf+k));
		printf("\n");
	}
#endif
	rtn = my_write(m_FD, m_tty, pBuf, dwLength);
	return rtn;
}

int MultiThrdSerial::readSerial(unsigned char * pBuf, u32 dwLength) {
	int nread;
//	printf("read, thread(%ld):", (long int)syscall(224));
//	printf("\n");
	nread = my_read(m_FD, m_tty, pBuf, dwLength);
	threadDebug
	if (nread > 0) {
		putDat(pBuf, nread);
	}

//	if (directConn() || connBy485()) {
//	} else {
//		nread = my_read(m_FD, m_tty, pBuf, dwLength);
//		if (nread > 0)
//			putDat(pBuf, nread);
//	}
	return nread;
}

bool MultiThrdSerial::serialOpen() {
	int ret;

	runCommProc();
	if (id ==0) {
		ret = pthread_create(&id, NULL, ComProc, this);
		if (ret < 0)
			return false;
		else
			return true;
	}
	return true;
}

bool MultiThrdSerial::serialClose() {
	quitCommProc();
	if (isatty(m_FD)) {
		close(m_FD);
		m_FD = -1;
		return true;
	} else
		return false;
}

extern GlobalDat gDat;
bool MultiThrdSerial::openPort(const char * portName, u32 tty, u32 sz)  {/*初始化串口*/
#if DEBUG_SCAN_DEV
	cout << "open dev:" << portName << endl;
#endif
	m_tty = tty;
//	if (connBy485()) {
//		m_FD = my_init(m_tty, gDat.baudRate485);
//	} else {
	if ((m_tty == 1) && (!canInitOpened)) {
		m_FD = my_open(portName, m_tty, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
		canInitOpened =  true;
	}
//	}

	if (directConn()) {
		m_FD = my_open(portName, m_tty, O_RDWR | O_NOCTTY /*| O_NDELAY*/);
		if (!isatty(m_FD)) {
			perror("open port error:");
			my_close(m_FD, m_tty);
			m_FD = -1;
			return false;
		}
	} else {
		if (m_FD == -1) {
			cout << "open can or 485 port error!" << endl;
			return false;
		}
	}
	return true;
}

bool MultiThrdSerial::config(int nSpeed, int nBits, char nEvent, int nStop) {
	struct termios newtio, oldtio;
	if (tcgetattr(m_FD, &oldtio) != 0) {
		perror("SetupSerial 1");
		exit(1);
	}
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch (nBits) {
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch (nEvent) {
	case 'O':
		newtio.c_cflag |= PARENB; //enble
		newtio.c_cflag |= PARODD;
		newtio.c_iflag |= INPCK ;
		break;
	case 'E':
		newtio.c_iflag |= (INPCK );
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}
	switch (nSpeed) {
	case 600:
		cfsetispeed(&newtio, B600);
		cfsetospeed(&newtio, B600);
		break;
	case 1200:
		cfsetispeed(&newtio, B1200);
		cfsetospeed(&newtio, B1200);
		break;
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 19200:
		cfsetispeed(&newtio, B19200);
		cfsetospeed(&newtio, B19200);
		break;
	case 38400:
		cfsetispeed(&newtio, B38400);
		cfsetospeed(&newtio, B38400);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}
	if (nStop == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if (nStop == 2)
		newtio.c_cflag |= CSTOPB;

	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 255;

	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_oflag &= ~OPOST;
	tcflush(m_FD, TCIFLUSH);
	if ((tcsetattr(m_FD, TCSANOW, &newtio)) != 0) {
		perror("com set error");
		return -1;
	}
	return true;

}

bool MultiThrdSerial::open_config(const char * portName, u32 ttyType, u32 Speed,
		u8 DatBits, const char *StopBits, char Pairy, u32 sz, bool bExt) {
	int nStop;
	if (0 == strcmp(StopBits, "1")) {
//		option.c_cflag &= ~CSTOPB; /* 1 stop bit */
		nStop = 1;
	} else if (0 == strcmp(StopBits, "1.5")) {
//		option.c_cflag &= ~CSTOPB; /* 1.5 stop bits */
		nStop = 1;
	} else if (0 == strcmp(StopBits, "2")) {
//		option.c_cflag |= CSTOPB; /* 2 stop bits */
		nStop = 2;
	} else {
//		option.c_cflag &= ~CSTOPB; /* 1 stop bit */
		nStop = 1;
	}
	clearBuf();
	m_portBuf = new unsigned char[sz];
	m_MaxSize = sz;
	if (ttyType == 2) {
//		return my_config(m_FD, ttyType, Speed, Pairy, DatBits, nStop);
		int rtn = my_config(m_FD, ttyType, Speed, Pairy, DatBits, nStop);
		usleep(300000);
		return rtn;
	} else {
		if (openPort(portName, ttyType, sz)) {
			if(0 == ttyType)
				return config(Speed, DatBits, Pairy, nStop) && serialOpen();
			else {
				int rtn = my_config(m_FD, ttyType, Speed, Pairy, DatBits, nStop) && serialOpen();
				usleep(300000);
				return rtn;
			}
//				return my_config(m_FD, ttyType, Speed, Pairy, DatBits, nStop) && serialOpen();
		}
	}
	return false;
}

bool MultiThrdSerial::setRaw(bool isRaw) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	if (isRaw) {
		option.c_iflag &= ~(ICRNL | IGNCR | INLCR | IGNBRK | BRKINT);
		option.c_iflag &= ~(IXON | IXOFF | IXANY);
		option.c_oflag &= ~(OCRNL | OLCUC | ONLCR | OPOST);
		option.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	} else {
		option.c_oflag |= OPOST;
		option.c_lflag |= (ICANON | ECHO | ECHOE | ISIG);
	}
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setMin(u32 min) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	option.c_cc[VMIN] = min;
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setTime(u32 delay) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	option.c_cc[VTIME] = delay;
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setSpeed(u32 speed) {
	if (!isatty(m_FD))
		return false;

	int i = 0;
	int table_size = 23;
	u32 speed_table[] = { 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800,
			2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
			500000, 576000, 921600 };
	int m_speed[] = { B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200,
	B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200,
	B230400, B460800, B500000, B576000, B921600 };
	for (i = 0; i < table_size; i++) {
		if (speed_table[i] >= speed)
			break;
	}
	if (i >= table_size)
		return false;
	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	if (cfsetispeed(&option, m_speed[i]) != 0)
		return false;
	if (cfsetospeed(&option, m_speed[i]) != 0)
		return false;
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setByteSize(u8 bytesize) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	option.c_cflag &= ~CSIZE;
	switch (bytesize) {
	case 5:
		option.c_cflag |= CS5;
		break;
	case 6:
		option.c_cflag |= CS6;
		break;
	case 7:
		option.c_cflag |= CS7;
		break;
	case 8:
		option.c_cflag |= CS8;
		break;
	default:
		option.c_cflag |= CS8;
		break;
	}
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setStopBits(const char *stopbit) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	if (0 == strcmp(stopbit, "1")) {
		option.c_cflag &= ~CSTOPB; /* 1 stop bit */
	} else if (0 == strcmp(stopbit, "1.5")) {
		option.c_cflag &= ~CSTOPB; /* 1.5 stop bits */
	} else if (0 == strcmp(stopbit, "2")) {
		option.c_cflag |= CSTOPB; /* 2 stop bits */
	} else {
		option.c_cflag &= ~CSTOPB; /* 1 stop bit */
	}
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

bool MultiThrdSerial::setParity(char parity) {
	if (!isatty(m_FD))
		return false;

	struct termios option;
	if (tcgetattr(m_FD, &option) != 0)
		return false;
	switch (parity) {
	case 'N':
		option.c_cflag &= ~PARENB;
		option.c_iflag &= ~INPCK;
		break;
	case 'E':
		option.c_cflag |= PARENB;
		option.c_cflag |= PARODD;
		option.c_iflag |= INPCK;
		break;
	case 'O':
		option.c_cflag |= PARENB;
		option.c_cflag &= ~PARODD;
		option.c_iflag |= INPCK;
		break;
	default:
		option.c_cflag &= ~PARENB;
		option.c_iflag &= ~INPCK;
		break;
	}
	if (tcsetattr(m_FD, TCSANOW, &option) != 0)
		return false;
	else
		return true;
}

