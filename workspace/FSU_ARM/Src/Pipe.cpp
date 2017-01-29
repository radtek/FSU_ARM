/*
 * Pipe.cpp
 *
 *  Created on: 2016-6-14
 *      Author: lcz
 */
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "Log.h"
#include "Pipe.h"
#include "LoginLogout.h"

#include "define.h"
using namespace std;

extern LoginHdl loginHdl;

void notifyCamera() {
	const char *fifo_name = "/tmp/fsu_fifo";
	int pipe_fd = -1;
	int res = 0;
	const int open_mode = O_RDWR;// O_RDONLY open error !!;

	if (access(fifo_name, F_OK) == -1) {
		//管道文件不存在
		res = mkfifo(fifo_name, 0666);
		if (res != 0) {
			char str[256];
			sprintf(str, "Could not create fifo %s\n", fifo_name);
			log(str);
			return;
		}
	}
	pipe_fd = open(fifo_name, open_mode);

	if (pipe_fd != -1) {
		char notify[2] = {1,0};
		res = write(pipe_fd, notify, 2);
		if (res == -1) {
			log("Write error on pipe\n");
		}
		close(pipe_fd);
	} else {
		log("pipe open error!\n");
	}
}
void notifyPPP(const char * fifo_name, const char * msg) {
//	const char *fifo_name = "/tmp/fsu_connect2D";
	int pipe_fd = -1;
	int res = 0;
	const int open_mode = O_WRONLY;

	if (access(fifo_name, F_OK) == -1) {
		//管道文件不存在
		res = mkfifo(fifo_name, 0666);
		if (res != 0) {
			char str[256];
			sprintf(str, "Could not create fifo %s\n", fifo_name);
			log(str);
			return;
		}
	}
	pipe_fd = open(fifo_name, open_mode);

	if (pipe_fd != -1) {
		const char * notify = msg;
		res = write(pipe_fd, notify, 2);
		if (res == -1) {
			log("Write error on pipe\n");
			return;
		}
		close(pipe_fd);
	} else {
		log("pipe open error!\n");
	}
}
bool getPipeNotify(const char * file, const char * o) {
	int pipe_fd = -1;
	u32 res = 0;
	char buffer[16] = {0};

	u32 len = 0;
	char * s = (char *) o;
	while(*s != 0) {
		len++;
		s++;
	}
	pipe_fd = open(file, O_RDWR);//O_RDONLY);
	if(pipe_fd != -1) {
		do {
			res = read(pipe_fd, buffer, PIPE_BUF);
			bool equ = true;
			if (res != len) {
				cout << "res != len" << endl;
				close(pipe_fd);
				return false;
			} else {
				for (u32 i = 0; i < len ; ++i) {
					if (buffer[i] != *(o + i)) {
						equ = false;
//						cout << "buffer[i] != *(o + i)" << buffer[i] << "vs" << *(o + i) << endl;
						break;
					}
				}
			}
			if (!equ) {
				cout << "unkown cmd form pipe file !" << endl;
			} else
				break;
		} while(res > 0);
		close(pipe_fd);
		MLOG("got msg - %s\n", buffer);
		return true;
	} else
		return false;
}
void checkRebootSignal() {
	while (1) {
		if (getPipeNotify("/tmp/fsu_2Fsu_reboot", "1")) {
			loginHdl.logout();
			log("system reboot by 4gDailer !!!\n");
			cout << endl << endl;
			cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << "$$$$$$$$$$$$$$$$$$$                                 $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << "$$$$$$$$$$$$$$$$$$$   system reboot by 4gDailer !!  $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << "$$$$$$$$$$$$$$$$$$$                                 $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
			cout << endl << endl << endl;
			sleep(30);
			system("reboot");
		}
	}
}




