/*
 * saveDatInTime.cpp
 *
 *  Created on: 2016-8-12
 *      Author: lcz
 */
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "saveDatInTime.h"
#include "Device.h"

extern DeviceManager DevMgr;
void timer_thread(union sigval v) {
#if DEBUG_HIS_DAT
	time_t t;
	t = time(0);
	printf("timer_thread function! %d - %s \n", v.sival_int, asctime(localtime(&t)));
#endif
	DevMgr.upDateHistoryDat();
}

bool initSaveHistoryDatProc() {
	// int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid);
	// clockid--值：CLOCK_REALTIME,CLOCK_MONOTONIC，CLOCK_PROCESS_CPUTIME_ID,CLOCK_THREAD_CPUTIME_ID
	// evp--存放环境值的地址,结构成员说明了定时器到期的通知方式和处理方式等
	// timerid--定时器标识符
	timer_t timerid;
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent)); //清零初始化

	evp.sigev_value.sival_int = 123; //也是标识定时器的，这和timerid有什么区别？回调函数可以获得
	evp.sigev_notify = SIGEV_THREAD; //线程通知的方式，派驻新线程
	evp.sigev_notify_function = timer_thread; //线程函数地址

	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
		perror("fail to timer_create");
		return false;
	}

	// int timer_settime(timer_t timerid, int flags, const struct itimerspec *new_value,struct itimerspec *old_value);
	// timerid--定时器标识
	// flags--0表示相对时间，1表示绝对时间
	// new_value--定时器的新初始值和间隔，如下面的it
	// old_value--取值通常为0，即第四个参数常为NULL,若不为NULL，则返回定时器的前一个值

	//第一次间隔it.it_value这么长,以后每次都是it.it_interval这么长,就是说it.it_value变0的时候会装载it.it_interval的值
	struct itimerspec it;
	it.it_interval.tv_sec = 1800; // 1800;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = 20;
	it.it_value.tv_nsec = 0;

	if (timer_settime(timerid, 0, &it, NULL) == -1) {
		perror("fail to timer_settime");
		return false;
	}

//    pause();

	return true;
}
/*
 * int timer_gettime(timer_t timerid, struct itimerspec *curr_value);
 * 获取timerid指定的定时器的值，填入curr_value
 *
 */

