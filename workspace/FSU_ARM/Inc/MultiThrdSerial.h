/*
 * MultiThrdSerial.h
 *
 *  Created on: 2016-4-17
 *      Author: lcz
 */
#ifndef _MULTITHREADSERIAL_H_
#define _MULTITHREADSERIAL_H_
#include <termios.h>
#include <mutex>
#include "define.h"
//#include "DevComm.h"
//#include "SerialPort.h"
//	class myMutex {
//		pthread_mutex_t can_writeLock;
//	};
//class SuperDevComm;
class DevComm;
class MultiThrdSerial {
public:
	MultiThrdSerial();
	virtual ~MultiThrdSerial();
	bool open_config(const char * portName, u32 ttyType,
			u32 Speed, u8 DatBits, const char *StopBits, char Pairy,
			u32 sz, bool bExDev);
	bool closePort() {return serialClose();}

	void setFD(int fd) {
		mtx_port.lock();
#if DEBUG_LOCK_485
		SHOW_TIME(" # 485 port lock [+] ");
#endif
		m_FD = fd;
//		cout << "m_FD=" << m_FD << endl;
	}
	void initSetFD(int fd) {
		m_FD = fd;
//		cout << "m_FD=" << m_FD << endl;
	}
	bool isMyFD(int &fd) {
		bool rtn = false;
		mtx_port.lock();
		rtn = (m_FD == fd);
		mtx_port.unlock();
		return rtn;
	}
	u32 writeSerial(unsigned char * pBuf, u32 dwLength); /*写串口数据*/
	int readSerial (unsigned char * pBuf, u32 dwLength); /*读串口数据*/
	bool openPort(const char * m_nComName, u32 ttys, u32 sz); /*初始化串口*/
	bool config( int nSpeed, int nBits,  char nEvent, int nStop)  ;
	bool serialOpen(); /*打开串口*/
	inline bool connBy485() { return (m_tty == 2); }
	inline bool directConn() { return (m_tty == 0); }
	inline int getTTY() { return m_tty; }
	void putDat(unsigned char * c, unsigned int len);
	void getBuf4Byte(unsigned char * out) {
		mtx_inDat.lock();
		for (u32 i = 0; i < 4; ++i)
			out[i] = m_portBuf[i];
		mtx_inDat.unlock();
	}
	void getBufCopy(unsigned char * out, unsigned int & len) {
		mtx_inDat.lock();
		for (u32 i = 0; i < m_pos; ++i)
			out[i] = m_portBuf[i];
		len = m_pos;
		mtx_inDat.unlock();
	}
	u32 pos() {
		u32 rtn = 0;
		mtx_inDat.lock();
//		cout << "\t[get pos +++";
		rtn = m_pos;
//		cout << " m_pos = " << m_pos;
//		cout << " --- ]\n";
		mtx_inDat.unlock();
		return rtn;
	}
	void clear() {
		mtx_inDat.lock();
//		cout << "\t[clr +++";
		m_portBuf[0] = 0;
		m_pos = 0;
//		cout << " comm port clear!";
//		cout << " --- ]" << endl;
		mtx_inDat.unlock();
		if (m_tty == 2) {
			mtx_port.unlock();
#if DEBUG_LOCK_485
			SHOW_TIME(" # 485 port lock [-] \n");
#endif
		}
	}
	bool isEmpty() {
		mtx_inDat.lock();
		int rtn = m_pos;
		mtx_inDat.unlock();
		return rtn == 0;
	}
	bool isNewPackIn(unsigned int &lastPos) {
		u32 p = pos();
		if (p > lastPos) {
//			cout << "add data, new pos =" << p << endl;
			lastPos = p;
			return true;
		}
		return false;
	}
	unsigned char * m_portBuf = NULL;
	unsigned int m_pos = 0;
	void setOwner(void * p) {
		RLock(mtx_owner);
		owner = p;
	}
	bool me(void * p) {
		bool r = false;
		mtx_owner.lock();
		r= owner==p;
		mtx_owner.unlock();
		return r;
	}
	bool isRunning() {
		bool rtn = false;
		mtx_run.lock();
		rtn = bRunning;
		mtx_run.unlock();
		return rtn;
	}
	void runCommProc() {
		mtx_run.lock();
		bRunning = true;
		mtx_run.unlock();
	}
	void quitCommProc() {
		mtx_run.lock();
		bRunning = false;
		mtx_run.unlock();
	}
private:
	void clearBuf();
	bool serialClose(); /*关闭串口*/
	bool setRaw(bool isRaw);
	bool setMin(u32 min);
	bool setTime(u32 delay);
	bool setSpeed(u32 speed);
	bool setByteSize(u8 bytesize);
	bool setStopBits(const char * stopbit);
	bool setParity(char parity);
private:
	pthread_t id = 0;
	unsigned int m_MaxSize = 0;
	std::mutex mtx_inDat;
	std::mutex mtx_port;	// !!!
	std::mutex mtx_owner;
	std::mutex mtx_run;
public:
	int m_FD = -1;
	int m_tty = 0;
	bool canInitOpened = false;
	bool bRunning = false;
	void * owner = NULL;
};

#endif //_MULTITHREADSERIAL_H_

