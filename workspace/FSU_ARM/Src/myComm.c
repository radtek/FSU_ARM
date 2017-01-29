/*
 * my_file.cpp
 *
 *  Created on: 2016-8-16
 *      Author: zuo
 */

/*
 * 使用说明
 * ttyS1-8：正常串口 ttyS9以上：模拟串口
 * tty_select 0：使用正常串口 1：使用模拟串口(CAN) 2:使用模拟串口(485)
 * fd:文件描述符 高4bits：扩展板地址 低4bits：（0：扩展板数据 1-4：串口1-4）
 */

/*
 * 串口说明 上面 1 4 8 5 下面 2 7 6 10
 */

/*
 * 备注:
 * 1.使用CAN时：
 * canid.addr
 * 当cid = 0x10（透传）时 高3bits：扩展板地址 低3bits：（0：扩展板数据 1-4：串口1-4）
 * 当cid = 0x07（设置参数）时 add：扩展板地址
 * 当cid = 0x08 (获取状态) 时 add：扩展版地址
 * baud ： B_table
 * parity ：（0：N，1：O，2：E）
 * stopbits：（0：1，1:1.5,2:2)
 *
 * 2.使用485时：
 * -----透传
 * 字头0xF5
 * 字尾 0xFB
 * 第1字节为字头
 * 第2个字节为canid.addr 高3bits：扩展板地址 低3bits：（0：扩展板数据 1-4：串口1-4）
 * 第3个字节为cid 0：透传 dtc：1串口设置
 * 第4个字节为数据长度(不计算字头 字尾 addr和cid)
 *
 * -----心跳包
 * 第1字节字头
 * 第2字节心跳包fd 0xFF
 * 第3字节为扩展板地址 0-8
 * 第4字节为字尾
 * 返回相同
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>             /* read, write, close */

#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <net/if.h>
#include<termios.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <linux/types.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <sys/time.h>

#include "user_can.h"
#include "user_led.h"
#include "myComm.h"

#define DLC 8 //can 数据长度
#define CAN_CID1 0x10 //
#define SET_CID1 0x07 //参数设置cid1
#define STATE_CID1 0x08 //获取状态
#define CAN_DTC 0x20 //设备类型
#define CAN_BAUT 250000 //can波特率
#define Baud_CID2 0x100 //波特率设置cid2
#define STATE_CID2 0 //获取状态cid2
#define false 0
#define true 1

#define SHOW_TIME(...) \
do {\
	timeval t;\
	gettimeofday(&t, NULL);\
	tm * p = localtime(&t.tv_sec);\
	fprintf(stdout, "[%02d:%02d:%02d %06ld] ", p->tm_hour, p->tm_min, p->tm_sec, t.tv_usec);\
	fprintf(stdout, __VA_ARGS__); \
}while(0)

#define fdHeart 0xFF

//串口透传cid
#define u2uCIDSend 0
#define u2uCIDSet 1
//字头 字尾
#define soi 0xF5
#define eoi 0xFB

//int FSUMode = 0; //fsu模式 1一体式 2分离式
extern int fsuMode;

int canfd[130]; // can 文件描述符
int canerrorfd;
int usartfd; //串口透传描述符

__u16 ctrlLed = 0xffff; //分离led控制 0灯亮 1灯灭
__u32 ctrlLedstm8 = 0xffffffff;  //一体led控制 0灯灭 1灯亮

pthread_mutex_t ledmutex; //led互斥锁
pthread_mutex_t myCommLock;

int tableLed[30];

//int canfd1, canfd2;
//int pack_id = 0; //packid 数据包id write 与 read的 id要相同

/*
 * CAN ID 格式
 */
typedef union _canid_ {
	__u32 Word; //字操作定义
	union _frame_format {
		struct _strFrameFm_ //标准帧结构
		{
			__u32 ID_BIT :11;
			__u32 RSV_BIT21 :21;
		} stcSTRFRFM;

		struct _etxFrameFm_ //扩展帧结构
		{
			__u32 ID_BIT :29;
			__u32 RSV_BIT3 :3;
		} stcETXFRFM;

		struct _etxdetailFm_ {
			__u32 Addr :6;
			__u32 CID2 :12;
			__u32 DTC :6;
			__u32 CID1 :5;
			__u32 RSV_BIT3 :3;
		} stcEtxDtFm;
	} FrameFm;
} uCANID, *P_uCANID;

/*
 * 提取字符串中的数字
 */
int strint(const char* str){
	int num=0, i=0;
	while(str[i]!='\0')	{
		if(str[i]>='0'&&str[i]<='9')//如果是数字
		num = num*10+(str[i]-'0');//连续数字转换为数.
		i++;
	}
	return num;
}

/*
 * led线程
 */
void *my_led(void* arg){
	int temp;
	while(1){
		if (fsuMode == 2){ //分离式 灯控
	//			pthread_mutex_lock(&ledmutex);
			HW_74HC595_Out(ctrlLed);
			ctrlLed = 0xffff;
			usleep(1000000);
//			HW_74HC595_Out(ctrlLed);
//			pthread_mutex_unlock(&ledmutex);
		}else if (fsuMode == 1) {//一体式灯控
//			pthread_mutex_lock(&ledmutex);
			HW_STM8_Out(ctrlLedstm8);
			ctrlLedstm8 = 0xffffffff;
			usleep(1000000);
//			HW_STM8_Out(ctrlLedstm8);
//			pthread_mutex_unlock(&ledmutex);
		}
	}
	return (void*)0;
}
/*
 * 根据fd，设置ctrlLed
 */

void set_ctrlLed(int fd, int select_tty, int state){
	int i, temp1 ,temp2;
	int shift = 0;

	//串口号
	int table[8] = {7, 6, 10, 1, 4, 8, 5, 2};
	int tablestm8[32] = {  //stm8灯 32led 先绿后红
							-1, -1, -1, -1, 1, -1, 8, -1,
							-1, -1, -1, -1, 7, -1, 4, -1,
							0x11, -1, 0x12, -1, 0x13, -1, 0x14, -1,
							0x01, -1, 0x02, -1, 0x03, -1, 0x04, -1,
						};
	if (fsuMode == 2){ //分离式 灯控
		for (i=0;i<30;i++){
			if (tableLed[i] == fd) {
				temp1 = i;
				break;
			}
		}
		for (i=0;i<8;i++){
			if (table[i] == temp1) {
				 temp2 = i;
				 break;
			}
		}
		if (state == 1){
			pthread_mutex_lock(&ledmutex);
			ctrlLed &=~ (0x8000 >> temp2);
//			HW_74HC595_Out(ctrlLed);
			pthread_mutex_unlock(&ledmutex);
		}else{
			pthread_mutex_lock(&ledmutex);
			ctrlLed |= (0x8000 >> temp2);
//			HW_74HC595_Out(ctrlLed);
			pthread_mutex_unlock(&ledmutex);
		}
	}else if (fsuMode == 1) {//一体式灯控
		if (select_tty == 0) {
			for (i=0;i<30;i++){
				if (tableLed[i] == fd) {
					temp1 = i;
					break;
				}
			}
			for (i=0;i<32;i++){
				if (tablestm8[i] == temp1) {
					shift = i;
					break;
				}
			}
		}else if(select_tty == 1){//扩展板灯控，一体式
			if ((fd & 0x0F) == 0) //获取扩展板本身数据不闪灯
				return;
			for (i=16;i<32;i++){
				if (tablestm8[i] == fd) {
					shift = i;
					break;
				}
			}
		}
		if (state == 1){
			pthread_mutex_lock(&ledmutex);
			ctrlLedstm8 &= ~(1 << shift);
//			HW_STM8_Out(ctrlLedstm8);
			pthread_mutex_unlock(&ledmutex);
		}else{
			pthread_mutex_lock(&ledmutex);
			ctrlLedstm8 |= (1 << shift);
//			HW_STM8_Out(ctrlLedstm8);
			pthread_mutex_unlock(&ledmutex);
		}
	}
}

/*
 * 计算modbus crc
 */
unsigned int getCRC16(const unsigned char *ptr, unsigned int len) {
	unsigned int wcrc = 0XFFFF; //预置16位crc寄存器，初值全部为1
	unsigned char temp; //定义中间变量
	unsigned int i = 0, j = 0; //定义计数
	for (i = 0; i < len; i++) //循环计算每个数据
			{

		temp = *ptr & 0X00FF; //将八位数据与crc寄存器亦或?
		ptr++; //指针地址增加，指向下个数据
		wcrc ^= temp; //将数据存入crc寄存器
		for (j = 0; j < 8; j++) //循环计算数据的
				{
			if (wcrc & 0X0001) //判断右移出的是不是1，如果是1则与多项式进行异或。
					{
				wcrc >>= 1; //先将数据右移一位
				wcrc ^= 0XA001; //与上面的多项式进行异或
			} else //如果不是1，则直接移出
			{
				wcrc >>= 1; //直接移出
			}
		}
	}
	unsigned int rtn = ((wcrc << 8) & 0xFF00) | ((wcrc >> 8) & 0xFF);

	return rtn; //crc的值

}

/*
 * 程 序 名: set_opt
 * 描    述: 透传串口设置, 115200,n,8,1
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */
int set_opt(int baud){
    struct termios newtio, oldtio;
    if (tcgetattr(usartfd, &oldtio) != 0){
        perror("SetupSerial 1");
        exit(1);
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    //数据位 8
    newtio.c_cflag |= CS8;

    //校验位 n
    newtio.c_cflag &= ~PARENB;

    //波特率
    switch (baud){
    	case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
		break;

    	case 57600:
			cfsetispeed(&newtio, B57600);
			cfsetospeed(&newtio, B57600);
		break;

    	case 38400:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
		break;

    	case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
		break;

    	default:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
		break;
    }

	//停止位 1
	newtio.c_cflag &= ~CSTOPB;

 	newtio.c_cc[VTIME] = 1;
	newtio.c_cc[VMIN] = 255;

    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_oflag &= ~OPOST;
    tcflush(usartfd, TCIFLUSH);

    if ((tcsetattr(usartfd, TCSANOW, &newtio)) != 0){
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

/*
 * 程序名: my_init
 * 描述 485和can初始化
 */

int my_init(int select_tty, int baud){
	struct sockaddr_can addr;
	struct ifreq ifr;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;

	if (fsuMode == 1)
		HW_STM8_Out(0xffffffff);
	else if (fsuMode == 2)
		HW_74HC595_Out(0xffff);//关灯

	can_set_bitrate("can0", CAN_BAUT);
	can_do_start("can0");

	strcpy(ifr.ifr_name, "can0");

	pthread_t led_thread;  //Led线程

	//第一次创建锁和初始化获取状态can

	pthread_mutex_init(&myCommLock, NULL); //创建互斥锁
	pthread_create(&led_thread, NULL, my_led, NULL);
	//can心跳包初始化
	canerrorfd = socket(family, type, proto);
	if (canerrorfd < 0) {
		printf("ERROR");
		return -1;
	}

	addr.can_family = family;
	if (ioctl(canerrorfd, SIOCGIFINDEX, &ifr) < 0) {
		printf("ERROR");
		return -1;
	}

	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(canerrorfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("ERROR");
		return -1;
	}

	if (select_tty == 2){

		usartfd = open("/dev/ttyS2",O_RDWR|O_NOCTTY);
		if(usartfd == -1){
			printf("cann't open usart \n");
			return 0;
		}
		set_opt(baud);

//		pthread_create(&led_thread, NULL, my_led, NULL);
//		pthread_mutex_init(&ledmutex, NULL);
		tableLed[2] = usartfd; //fd与串口号 对应表
		return usartfd;
	}
}

/*
 * 程 序 名: my_open
 * 描    述: my_open
 * 入口参数:
 * 返回参数: fd
 * BY:
 * DATE:
 */
int my_open(const char* pathname, int select_tty, int oflag) {
	int res, num;

	struct sockaddr_can addr;
	struct ifreq ifr;
	int family = PF_CAN, type = SOCK_RAW, proto = CAN_RAW;

//	pthread_t led_thread;  //Led线程

	if (select_tty == 0) {

		/*正常串口tty1-8*/
//		printf("open tty: %s\n", pathname);
		res = open(pathname, oflag);
//		printf("open return: %d\n", res);
		num = strint(pathname);
		if (num < 30) //防止表溢出
			tableLed[num] = res; //fd与串口号 对应表
//		printf("usart num is :-->%d\n", num);

	} else {
		if (select_tty == 2)
			return usartfd;

		strcpy(ifr.ifr_name, "can0");
		num = atoi(pathname);
		if (num == 485){
//			printf("%d",num);
			return num;
		}
		num = num / 10 * 16 + num % 10;
		res = num;

		canfd[num] = socket(family, type, proto);
		if (canfd[num] < 0) {
			printf("ERROR");
			return -1;
		}

		addr.can_family = family;
		if (ioctl(canfd[num], SIOCGIFINDEX, &ifr) < 0) {
			printf("ERROR");
			return -1;
		}

		addr.can_ifindex = ifr.ifr_ifindex;
		if (bind(canfd[num], (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			printf("ERROR");
			return -1;
		}
		printf("CANset,ok\n");
	}

	return res;
}

/*
 * 程 序 名: my_close
 * 描    述: my_close
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */
int my_close(int fd, int select_tty) {
	int res;

//	pthread_mutex_destroy(&myCommLock);
	if (select_tty == 0)
		res = close(fd);
	else
		res = 0;

	return res;
}

int BaudSet(int baud) {
	int B_table[] = { 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
	__u8 i;

	for (i = 0; i < sizeof(B_table); i++) {
		if (baud == B_table[i])
			return i;
	}

	return -1;
}

/*
 * 程 序 名: my_config
 * 描    述: 虚拟串口设置
 * 入口参数:
 * 返回参数: -1 失败，0 成功
 * BY:
 * DATE:
 */
int my_config(int fd, int select_tty, int baud, char parity, int len, float stopbits) {
	int _baud, _parity, _len, _stopbits;
	int ret, ret1;
	unsigned char sendBuf[500];

	uCANID canid;
	struct can_frame frame;

	//转换参数
	_baud = BaudSet(baud);
	if (_baud == -1)
		return 0;

	if (parity == 'N')
		_parity = 0;
	else if (parity == 'O')
		_parity = 1;
	else if (parity == 'E')
		_parity = 2;
	else
		return 0;

	_len = len;

	if (stopbits == 1)
		_stopbits = 0;
	else if (stopbits == 1.5)
		_stopbits = 1;
	else if (stopbits == 2)
		_stopbits = 2;
	else
		return 0;

	//test
	//printf("baud:%d,parity:%d,stopbits:%d\n",_baud,_parity,_stopbits);

	//设置can id
	if (select_tty == 1) {
		canid.FrameFm.stcEtxDtFm.CID1 = SET_CID1;
		canid.FrameFm.stcEtxDtFm.DTC = CAN_DTC;
		canid.FrameFm.stcEtxDtFm.CID2 = Baud_CID2;
		canid.FrameFm.stcEtxDtFm.Addr = (fd & 0x70) >> 4;

		frame.can_dlc = DLC;
		frame.can_id = canid.Word;
		frame.can_id &= CAN_EFF_MASK;
		frame.can_id |= CAN_EFF_FLAG;

		frame.data[0] = (__u8) (fd & 0x07);
		frame.data[1] = (__u8) _baud;
		frame.data[2] = (__u8) _parity;
		frame.data[3] = (__u8) _len;
		frame.data[4] = (__u8) _stopbits;

		ret1 = 0;
		while (ret1 < sizeof(frame)) {
			ret = write(canfd[fd], &frame, sizeof(frame));
			if (ret == -1) {
				printf("write error !! \n");
				return 0;
			}
			ret1 += ret;
		}
	} else if (select_tty == 2) {
		sendBuf[0] = soi;
		sendBuf[1] = ((fd & 0x70) >> 1) | (fd & 0x07);
		sendBuf[2] = u2uCIDSet;
		sendBuf[3] = 5;

		sendBuf[4] = fd & 0x07;
		sendBuf[5] = _baud;
		sendBuf[6] = _parity;
		sendBuf[7] = _len;
		sendBuf[8] = _stopbits;

		sendBuf[9] = eoi;
		//加2字节保证全部发送，bug
//		tcflush(usartfd, TCOFLUSH);
		write(usartfd, sendBuf, 10);
	}

	return 1;
}

/*
 * 程 序 名: my_write
 * 描    述: 虚拟串口设置
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */
int my_write(int fd, int select_tty, unsigned char *buf, int count) {
	uCANID wcanid;
	struct can_frame wframe;
	int i, j, temp, pack_sid, pack_id;
	int ret, ret1, rtn;
	unsigned char sendBuf[500];

/*
	for (i=0;i<10;i++)
		printf("0x%x\n",*(buf+i));
*/
//	pack_id = (pack_id+1)%8;

	if (select_tty == 0) {
//		unsigned int k;
//		printf("-->>-->>-->> ");
//		for (k = 0; k < count; ++k)
//			printf(" %02X", *(buf+k));
//		printf("\n");

//		set_ctrlLed(fd, select_tty, 1); // 灯闪
		rtn =  write(fd, buf, count);
//		set_ctrlLed(fd, select_tty, 0); // 灯闪
		return rtn;
	} else if (select_tty == 1){
		pthread_mutex_lock(&myCommLock);
		wcanid.FrameFm.stcEtxDtFm.CID1 = CAN_CID1;
		wcanid.FrameFm.stcEtxDtFm.DTC = CAN_DTC;
		wcanid.FrameFm.stcEtxDtFm.Addr = ((fd & 0x70) >> 1) | (fd & 0x07);
		wframe.can_dlc = DLC;

		for (i = 0; i <= (count - 1) / 8; i++) {
			memset(wframe.data, 0, 8);
			pack_sid = i;
			temp = count - i * 8;
			pack_id = (count - 1) % 8;  //pack_id 取长度余8 总长度pack_sid * 8 + pack_id
			if (temp >= 8)
				temp = 8;
//			memcpy(wframe.data,buf+i*8,temp);
			for (j = 0; j < temp; j++){
				wframe.data[j] = *(buf + i * 8 + j);
//				printf("%x,%d,%d\n",wframe.data[j],i,j);
			}

			//最后一子包加标志位
			if (i == (count - 1) / 8) {
				pack_sid |= 0x80;
			}

			wcanid.FrameFm.stcEtxDtFm.CID2 = (pack_id << 8) | pack_sid;

			wframe.can_id = wcanid.Word;
			wframe.can_id &= CAN_EFF_MASK;
			wframe.can_id |= CAN_EFF_FLAG;

			ret1 = 0;
			while (ret1 < sizeof(wframe)) {
//				set_ctrlLed(fd, select_tty, 1); // 灯闪
				ret = write(canfd[fd], &wframe, sizeof(wframe));
//				set_ctrlLed(fd, select_tty, 0); // 灯闪
				if (ret == -1) {
					printf("write error !! \n");
					return -1;
				}
				ret1 += ret;
			}
			usleep(3000);
		}
		pthread_mutex_unlock(&myCommLock);

	}else if (select_tty == 2){

		if (fd==485){
			sendBuf[0] = soi;
			sendBuf[1] = fdHeart;
			sendBuf[2] = *buf;
			sendBuf[3] = eoi;
			set_ctrlLed(usartfd, 0, 1); // 灯闪
			write(usartfd, sendBuf, 4);
			set_ctrlLed(usartfd, 0, 0); // 灯闪
//			printf("-->-->send data:");
//				for (i=0;i<4;i++)
//					printf("%02x ",sendBuf[i]);
//				printf("\n");
		}else{
			sendBuf[0] = soi;
			sendBuf[1] = ((fd & 0x70) >> 1) | (fd & 0x07);
			sendBuf[2] = u2uCIDSend;
			sendBuf[3] = count;
			memcpy(&sendBuf[4], buf, count);
			sendBuf[4+count] = eoi;
			//加2字节保证全部发送，bug
	//		tcflush(usartfd, TCOFLUSH);
//			set_ctrlLed(usartfd, 0, 1); // 灯闪
			write(usartfd, sendBuf, count+5);
//			set_ctrlLed(usartfd, 0, 0); // 灯闪
//			printf("-->-->send data:");
//			for (i=0;i<count+5;i++)
//				printf("%02x ",sendBuf[i]);
//			printf("\n");
		}

//		write(usartfd,"0123456789012345678901234567890123456789\n",40);
	}

	return count;
}

/*
 * 程 序 名: my_read
 * 描    述: 虚拟串口设置
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */
int my_read(int fd, int select_tty, unsigned char *buf, int count) {
	uCANID rcanid;
	struct can_frame rframe;
	int j, pack_sid, pack_id;
	int ret, ret1;
	int rcount = 0; //接收正确包数
	int rcvcount = 0; //接收包数
	unsigned char receiveBuf[500];
	int rtn = 0; //通过usart透传时，返回的长度

	if (select_tty == 0){
		rtn =  read(fd, buf, count);
		if (rtn > 0) {
			set_ctrlLed(fd, select_tty, 1); // 灯闪
//			usleep(100000);
//			set_ctrlLed(fd, select_tty, 0); // 灯闪
		}
		return rtn;
	}
	else if (select_tty == 1){
//		printf("one while\n");
//		while (1) {
			ret1 = 0;
			while (ret1 < sizeof(rframe)) {
//				set_ctrlLed(fd, select_tty, 1); // 灯闪
				ret = read(canfd[fd], &rframe, sizeof(rframe));
//				set_ctrlLed(fd, select_tty, 0); // 灯闪
				if (ret == -1) {
					printf("read error !! \n");
					return -1;
				}
				ret1 += ret;
				rcvcount++;
			}

			rcanid.Word = rframe.can_id;

			pack_id = (rcanid.FrameFm.stcEtxDtFm.CID2 >> 8) & 0x0F;
			pack_sid = rcanid.FrameFm.stcEtxDtFm.CID2 & 0xFF;

			//test
//			printf("*****my_read: %x,%x,%x,%x,%x,%x\n",
//					fd,
//					rcanid.FrameFm.stcEtxDtFm.DTC,
//					rcanid.FrameFm.stcEtxDtFm.Addr,
//					pack_id,
//					rcanid.FrameFm.stcEtxDtFm.CID1,
//					pack_sid);

			if ((CAN_DTC == rcanid.FrameFm.stcEtxDtFm.DTC)
					&& (rcanid.FrameFm.stcEtxDtFm.CID1 == CAN_CID1)
//					&& (t_pack_id == pack_id)
					&& (((((fd & 0x70) >> 1) | (fd & 0x07))
							== rcanid.FrameFm.stcEtxDtFm.Addr) )) {
				//test
				//printf("rcv,ok\n");
//				SHOW_TIME("before XXXXXXXXXX\n");

//				printf("read buf is :");
//				for (int k=0;k<8;k++)
//					printf("%02X,", rframe.data[k]);
//				printf("\n");

//				printf("myRead: firstrcout : %d\n", rcount);
				if ((pack_sid & 0x80) == 0) {
					for (j = 0; j < 8; j++)
						*(buf + pack_sid * 8 + j) = rframe.data[j];
					rcount++;
//					printf("myRead: rcout : %d\n", rcount);
					rcvcount--;
					return 0;
				}else{ //最后一包
					//test
					//printf("last:%d,%d",rcount,pack_sid & 0x7F);
					for (j = 0; j < 8; j++)
						*(buf + (pack_sid & 0x7F) * 8 + j) = rframe.data[j];

//					printf("myRead: rcoutlast : %d\n", rcount);
//					if ((pack_sid & 0x7F) != rcount)
//						return 0;
					set_ctrlLed(fd, select_tty, 1); // 灯闪
//					SHOW_TIME("after XXXXXXXXXX\n");
					usleep(1000000);
					rtn = ((pack_sid & 0x7F)) * 8 + pack_id + 1;
//					set_ctrlLed(fd, select_tty, 0); // 灯闪
					return rtn;
				}
			}

			//如果接收10包不正确以上
//			if (rcvcount >= 10)
//				return -1;
//		}
	}else if (select_tty == 2){

//		set_ctrlLed(usartfd, 0, 1); // 灯闪

		rtn = read(usartfd, buf, count);
		if (rtn > 0) {
			set_ctrlLed(usartfd, 0, 1); // 灯闪
//			usleep(100000);
//			set_ctrlLed(fd, select_tty, 0); // 灯闪
		}
//		set_ctrlLed(usartfd, 0, 0); // 灯闪
		return rtn;
		if (ret == -1) {
			printf("read error!\n");
			return -1;
		}
//		printf("soi:0x%x",receiveBuf[0]);
		if (receiveBuf[0] != soi)
			rtn |= 1;
//		printf("soi,ok\n");

		if (fd==485) {
//			read(usartfd,&receiveBuf[1],1);
			if (receiveBuf[1] != fdHeart)
				rtn |= 1;

			if (rtn!=0){
				tcflush(usartfd, TCIFLUSH);
				return 0;
			}

//			read(usartfd,&receiveBuf[2],1);
//			read(usartfd,&receiveBuf[3],1);
			if (receiveBuf[3] != eoi){
				tcflush(usartfd, TCIFLUSH);
				return 0;
			}

			*buf = receiveBuf[2];
			tcflush(usartfd, TCIFLUSH);
			return 1;

		} else {
//			read(usartfd,&receiveBuf[1],1);
			if (receiveBuf[1] != (((fd & 0x70) >> 1) | (fd & 0x07)))
				rtn |= 1;

//			read(usartfd,&receiveBuf[2],1);
			if (receiveBuf[2] != u2uCIDSend)
				rtn |= 1;

			if (rtn!=0){
				tcflush(usartfd, TCIFLUSH);
				return 0;
			}

//			read(usartfd,&receiveBuf[3],1);
			rtn = receiveBuf[3];
/*
			ret1 = 0;
			while (ret1 < rtn) {
				ret = read(usartfd,&receiveBuf[4+ret1],rtn);
				printf("%d\n",ret);
				if (ret == -1) {
					printf("read error !! \n");
					return -1;
				}
				ret1 += ret;
			}*/

//			read(usartfd,&receiveBuf[rtn+4],1);
			if (receiveBuf[rtn+4] != eoi){
				tcflush(usartfd, TCIFLUSH);
				return 0;
			}

			memcpy(buf, &receiveBuf[4], rtn);
			tcflush(usartfd, TCIFLUSH);
			return rtn;
		}
	}
	return 0;
}

/*
 * 程 序 名: my_transform
 * 描    述: 虚拟串口设置
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */

int my_transform(int fd, int select_tty,
		unsigned char *rb,	// receive buffer
		unsigned int * pos,
		unsigned char *buf, unsigned int count){
	char str[256];
	char temp[256]; //读取文件字符
	int DItable[8] = {230, 231, 228, 229, 234, 235, 232, 233};
	int rtn = 1, len = 0;
	int i;
	int envmod = 1; //当使用can，获取采集板数据时，需要和主板采集的合并
	unsigned int crc;

	// RS485 分离式 tty=2
	if (select_tty == 2){
		if (count < 4)
			return false;

		if (buf[0] != soi)
			return false;

		if (fd==485){
			if ((buf[1] == fdHeart) && (buf[3] == eoi)) {
				*rb = buf[2];
				*pos = 1;
			} else
				return false;
		}else{
			if (buf[1] != (((fd & 0x70) >> 1) | (fd & 0x07)))
				return false;
			if (buf[2] != u2uCIDSend)
				return false;

			if (buf[buf[3] + 4] != eoi)// len was *count
				return false;
			*pos = buf[3];
			memcpy(rb, &buf[4], *pos);
		}
		tcflush(usartfd, TCIFLUSH);
	//	printf("tcflush(), %d\n", usartfd);
		return true;
	}else if (select_tty == 1){// can一体式 tty=1
		if ((fd != 0) && (fd != 0x10))
			envmod &= 0;
		if (buf[0] != 1)  //modbus地址
			envmod &= 0;
		if (buf[1] != 3) //modbus功能码
			envmod &= 0;
		if (buf[2] != 32) //长度 16个数据*2 32
			envmod &= 0;

		if (envmod == 1) {//如果是获取采集板数据
			memcpy(rb, buf, 35);
			*(rb+2) = buf[2] + 16; //长度加上主板的8个DI量
			for (i=0;i<8;i++) {
				*(rb+35+i*2) = 0;  //modbus寄存器16位数据，高8位填0
				sprintf(str, "/sys/class/gpio/gpio%d/value", DItable[i]);
				fd = open(str, O_RDONLY);
				read(fd, temp, 1);
				//文件读出的数据是ascii码 ，需要转换为数值
				if (temp[0] == '0')
					*(rb+i*2+35+1) = 1;
				else if (temp[0] == '1')
					*(rb+i*2+35+1) = 0;
//				*(rb+i*2+35+1) = atoi(temp);
				close(fd);
			}
			// 35+2*(8-1)+1 = 50
			crc = getCRC16(rb, 51);
			*(rb+51) = (crc >> 8) & 0xFF;
			*(rb+52) = crc & 0xFF;
//			for (i=0;i<53;i++)
//				printf("%02x,", *(rb+i));
			*pos = 53;
		} else { //透传数据
			memcpy(rb, buf, count);
			*pos = count;
		}
	}
	return rtn;
}

/*
int my_transform(int fd,
		unsigned char *rb,	// receive buffer
		unsigned int * pos,
		unsigned char *buf, unsigned int count){
	if (count < 4)
		return false;

	if (buf[0] != soi)
		return false;

	if (fd==485){
		if ((buf[1] == fdHeart) && (buf[3] == eoi)) {
			*rb = buf[2];
			*pos = 1;
		} else
			return false;
	}else{
		if (buf[1] != (((fd & 0x70) >> 1) | (fd & 0x07)))
			return false;
		if (buf[2] != u2uCIDSend)
			return false;

		if (buf[buf[3] + 4] != eoi)// len was *count
			return false;
		*pos = buf[3];
		memcpy(rb, &buf[4], *pos);
	}
	tcflush(usartfd, TCIFLUSH);
//	printf("tcflush(), %d\n", usartfd);
	return true;
}
*/

/*
 * 程 序 名: heartLoop
 * 描    述: 判断是否故障
 * 入口参数:
 * 返回参数:
 * BY:
 * DATE:
 */

int heartLoop(void) {

	int ret, ret1;
	uCANID scanid;
	struct can_frame sframe;
	__u8 CommByte = 0;
	int i;

	ret1 = 0;
	while (ret1 < sizeof(sframe)) {

		ret = read(canerrorfd, &sframe, sizeof(sframe));
		if (ret == -1) {

			printf("get state error !! \n");
			return -1;
		}
		ret1 += ret;
	}

	scanid.Word = sframe.can_id;

	if ((scanid.FrameFm.stcEtxDtFm.CID1 == STATE_CID1)
			&& (scanid.FrameFm.stcEtxDtFm.CID2 == STATE_CID2)
			&& (scanid.FrameFm.stcEtxDtFm.DTC == CAN_DTC)) {

//		printf("%d\n",scanid.FrameFm.stcEtxDtFm.Addr);
		CommByte = scanid.FrameFm.stcEtxDtFm.Addr;
//		for (i = 0; i < 8; i++) {
//
//			CommByte >>= 1;
//
//			if (i == scanid.FrameFm.stcEtxDtFm.Addr)
//				CommByte |= 0x80;
//		}
//		commErrClear(scanid.FrameFm.stcEtxDtFm.Addr);
		return CommByte;
	}
	return 0;
}

/*
int main(int argc, char* argv[]) {
	int res, res1, flag;
	__u8  rbuf[100], i;
	unsigned char buf[100];
	int temp, count;

	FSUMode = 1;
	for (i = 0; i < 100; i++)
		buf[i] = i;

	my_init(1,0);
	res = my_open("21", 1, 1);
//	res1 = my_open("485", 2, 1);
	res1 = my_open("/dev/ttyS4", 0, 1);
	printf("%d\n", res);
	printf("%d\n", canfd[res]);

//	my_config(res, 2, 9600, 'N', 8, 1);
//	sleep(3);

	//扩展板buf 01 03 00 00 00 10 44 06 | 02 03 00 00 00 10 44 35
	buf[0] = 0x02;
	buf[1] = 0x03;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = 0x00;
	buf[5] = 0x10;
	buf[6] = 0x44;
	buf[7] = 0x35;

	//控制继电器
//	buf[0] = 0x01;
//	buf[1] = 0x06;
//	buf[2] = 0x01;
//	buf[3] = 0x01;
//	buf[4] = 0x00;
//	buf[5] = 0x00;
//	buf[6] = 0xD9;
//	buf[7] = 0xf6;

	buf[8] = 3;

	flag = 0;

	 while(1){
		 if (flag==0)
			 my_write(res,1,buf,8);
		 else
//			 my_write(res1,2,&buf[8],1);
			 my_write(res1,0,buf,8);
//		 flag = (flag+1)%2;

//		 usleep(10000);
		 count = my_read(res, 1, rbuf, 100);
//		 count = my_read(res, 0, rbuf, 256);
		 if (count!=-1){
			 printf("receive num:::::%d\n",count);
			 printf("-->-->receive data:");
			 for (i=0;i<count;i++)
				 printf("%02x-",rbuf[i]);
			 printf("\n");
		 }
		 temp = my_transform(res1, 1, rbuf, &count);
		 printf("return:%d\n", temp);

		 if (temp){
			 printf("receive num:::::%d\n",count);
			 printf("-->-->receive data:");
			 for (i=0;i<count;i++)
				 printf("%02x,",rbuf[i]);
			 printf("\n");
		 }
//		 usleep(300000);
		 sleep(1);
	 }

//	 struct can_frame frame;

//	 ret = read(canfd1, &frame, sizeof(frame));
//	 printf("x1\n");
//	 ret = read(canfd2, &frame, sizeof(frame));
//	 printf("x2\n");
//	 return 0;


	while (1) {
		res = heartLoop();
		printf("0x%x\n", res);
	}
	return 0;
}
*/


