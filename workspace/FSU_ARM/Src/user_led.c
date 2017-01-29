/*
 * FSU_Led.c
 *
 *  Created on: 2016-9-28
 *      Author: vmuser
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

#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <net/if.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <sys/uio.h>

#include <linux/types.h>

#include <linux/rtnetlink.h>
#include <linux/netlink.h>
//#include <linux/gpio.h>

#define rckNum 0x6A //PD10
#define sckNum 0x6B //PD11
#define sdaNum 0x6C //PD12

int initLed(void){
	FILE *fp;
	char str[256];
	int portNum[3]={rckNum, sckNum, sdaNum};
	int i;

	if ((fp = fopen("/sys/class/gpio/export", "w")) == NULL) {
		printf("Cannot open export file.\n");
		return -1;
	}
	for (i=0;i<3;i++){
		fprintf(fp, "%d", portNum[i]);

		//  linux equivalent code "echo in > direction" to set the port as an input
		sprintf(str,"/sys/class/gpio/gpio%d/direction", portNum[i]);
		if ((fp = fopen(str, "rb+")) == NULL) {
			printf("Cannot open direction file.portnum: %d\n", portNum[i]);
			return -1;
		}
		fprintf(fp, "out");
		fclose(fp);
	}
	return 0;
}

int ctrlPort(int portNum, int ctrl){
	FILE *fp;
	char str[256];
	sprintf(str,"/sys/class/gpio/gpio%d/value",portNum);
	fp = fopen(str, "rb+");
	if (fp == NULL) {
//		printf("Cannot write value.portnum: %d\n", portNum);
		return -1;
	}
	if (ctrl)
		fprintf(fp, "1");
	else
		fprintf(fp, "0");
	fclose(fp);
	return 0;
}

void HW_74HC595_Send(__u8 data){
	__u8 i;
	for (i=0;i<8;i++)	{
		if (data & 0x80)
			ctrlPort(sdaNum, 1);
		else
			ctrlPort(sdaNum, 0);
		ctrlPort(sckNum, 0);
		usleep(5);

		data<<=1;
		ctrlPort(sckNum, 1);
		usleep(5);
	}
}

void HW_74HC595_Out(__u16 data){
	__u8 a, b;
	a = (__u8)data;
	b = (__u8)(data>>8);
	HW_74HC595_Send(a);
	HW_74HC595_Send(b);
	ctrlPort(rckNum, 0);
	usleep(5);
	ctrlPort(rckNum, 1);
}

void HW_STM8_Out(__u32 data){
	__u8 a, b, c, d;
	d = (__u8)data;
	c = (__u8)(data>>8);
	b = (__u8)(data>>16);
	a = (__u8)(data>>24);
	HW_74HC595_Send(a);
	HW_74HC595_Send(b);
	HW_74HC595_Send(c);
	HW_74HC595_Send(d);
	ctrlPort(rckNum, 0);
	usleep(5);
	ctrlPort(rckNum, 1);
}
