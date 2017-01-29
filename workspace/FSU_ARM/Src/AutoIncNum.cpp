/*
 * AutoIncNum.cpp
 *
 *  Created on: 2016年9月10日
 *      Author: root
 */
#include <stdio.h>
//#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include "AppConfig.h"
#include "AutoIncNum.h"

using namespace std;

bool AutoIncNum::init() {
	FILE * stream;
	stream = fopen(pf, "rb");
	bool rtn = true;
	if (stream == NULL) {	// 文件不存在
		FILE * stream1;
		stream1 = fopen(pf, "wb");
		if (stream1 == NULL) {
			perror ("fail to open and create counter file !");
			rtn = false;
		} else {
			unsigned char buf[4] = {0};
			int n = fwrite(buf, sizeof(unsigned char), 4, stream1);
			if (n != 4) {
				cout << "idx file error !" << endl;
				rtn = false;
			} else {
				MLOG("$$$ 1st almIdx = %d\n", idx);
				idx = 0;
			}
			fclose(stream1);
		}
	} else {
		unsigned char buf[4];
		int k = fread(buf, sizeof(unsigned char), 4, stream);
		if (k != 4) {
			log ("read error of alm counter file when init...");
			rtn = false;
		} else {
			unsigned int * p = (unsigned int *)buf;
			idx = *p;
#if DEBUG
			cout << "$$$ init almIdx = " << idx << endl;
#endif
		}
		fclose(stream);
	}
	return rtn;
}
AutoIncNum::AutoIncNum() {
	idx = 0;
	char path[256];
	string exePath = get_exe_path(path, 256);
	fn = exePath + string("ain.dat");
	pf = fn.c_str();
}
bool AutoIncNum::add(unsigned int n, unsigned int &out) {
	mtx.lock();
	FILE * stream;
	stream = fopen(pf, "r");
	bool rtn = true;
	if (stream == NULL) {
//		FILE * stream1;
//		stream1 = fopen(pf, "wb");
//		if (stream1 == NULL) {
//			log ("fail to open and create counter file !");
//			rtn = false;
//		} else {
//			unsigned char buf[4] = {0};
//			int n = fwrite(buf, sizeof(unsigned char), 4, stream1);
//			if (n != 4) {
//				log ("counter file data error!");
//				rtn = false;
//			} else {
//				idx = 0;
//				cout << "$$$ init almIdx = " << idx << endl;
//			}
//			fclose(stream1);
//		}
		rtn = false;
	} else {
		unsigned char buf[4];
		int k = fread(buf, sizeof(unsigned char), 4, stream);
		fclose(stream);
		if (k != 4) {
			log ("read error of alm counter file whe add new !");
			rtn = false;
		} else {
			unsigned int * pi = (unsigned int *)buf;
			unsigned old = *pi;	// read old
			*pi = *pi + n;	// change buf.
			//------------------------------
			stream = fopen(pf, "w");
			if (stream == NULL) {
				log ("open to alm counter file error when add new !");
				rtn = false;
			} else {
				fseek(stream, 0, SEEK_SET);
				k = fwrite(buf, sizeof(unsigned char), 4, stream);// write buf to file
				if (k != 4) {
					log ("write alm counter file error when add new !");
					rtn = false;
				} else {
					idx = *pi;
					out = old;
				}
			}
			fclose(stream);
		}
	}
	mtx.unlock();
	return rtn;
}

