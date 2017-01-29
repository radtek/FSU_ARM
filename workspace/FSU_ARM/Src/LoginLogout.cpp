/*
 * LoginLogout.cpp
 *
 *  Created on: 2016-8-13
 *      Author: lcz
 */
#include <string.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <time.h>
#include <mutex>
#include "AppConfig.h"
#include "LoginLogout.h"
#include "Pipe.h"
#include "Message.h"
#include "soap.h"
#include "define.h"

using namespace std;
extern GlobalDat gDat;
extern SCCallProxy scProxy;
extern LoginHdl loginHdl;
extern RunStat AllRun;

bool LoginHdl::login2SC() {

	string str2SC;
	string strSCResponse;
#if DEBUG
	cout << "\nlogging...\n" << endl;
//	cout << str2SC << endl;
#endif
	ipaddr(gDat.myIp);
	cout << endl << "My IP : " << gDat.myIp.c_str() << endl << endl;
	scProxy.getLoginPak(str2SC);
	if(!SCLogin_Invoke(str2SC, strSCResponse)) {
#if DEBUG
		cout << "\nlogin failed!\n" << endl;
#endif
		return false;
	}
	MessageMgr::hdlScResponse(strSCResponse);

	return true;
}
bool LoginHdl::logout() {
	if (!isLogin())
		return false;
	string str2SC;
	string strSCResponse;
	scProxy.getLogoutPak(str2SC);
#if DEBUG
	cout << "logout..." << endl;
	cout << str2SC << endl;
#endif
	if(!SCLogin_Invoke(str2SC, strSCResponse))
		return false;
	MessageMgr::hdlScResponse(strSCResponse);

//	notifyPPP("/tmp/fsu_2Dialer_logout_offline", "1");
	system("/etc/ppp/peers/scripts/vpn_off");

	sleep(2);

	return true;
}
extern int NetMode;
void LoginHdl::setLoginState(bool b) {
	RLock(mtx_login);
	cout << "set login state = " << (u32) b << endl;
	bLogin = b;
}
void* vpnOnlineChkProc(void * param) {
	SET_THRD_NAME();
	LoginHdl *p = (LoginHdl*) param;
	while(AllRun.isRunning()) {
		p->setVpnOnline(getPipeNotify( "/tmp/fsu_connectState", "online"));
		sleep(1);
	}
	return (void *)0;
}
void* loginHdlProc(void * param) {
	SET_THRD_NAME();
	LoginHdl *p = (LoginHdl*) param;
	while(AllRun.isRunning()) {
		if (NetMode == 0) {
			if (!p->isLogin()) {
				if (p->login2SC()) {
//					p->setLoginState(true);
					p->setScOnline(true);
				}
			} else {

			}
			sleep(10);
		} else {	// vpn
			if (p->isVpnOnline()) {
				bool bLoginOk = false;
				if (!p->isLogin()) {
					for (u32 i = 0; i < 2; ++i) {
						if (p->login2SC()) {
							p->m_last = system_clock::now();
							p->setLoginState(true);
							p->setScOnline(true);
							bLoginOk = true;
							break;
						}
						sleep(60);
					}
					if(!bLoginOk) {
						notifyPPP("/tmp/fsu_2Dialer_recall", "1");// 通知断网了
						sleep(10);
					}
				}
			}
		}
	}
	return (void *)0;
}
void * ScOnlineChkProc(void * param) {
	SET_THRD_NAME();
	LoginHdl *p = (LoginHdl*) param;
	while(AllRun.isRunning()) {
//		cout << "tick\n";
		if (p->isScOnline()) {
			auto now = system_clock::now();
			auto dur = duration_cast < milliseconds > (now - p->m_last);
			u32 cntHeartBreak = p->getHeartBreakCnt();
#if DEBUG_SC_HEART_BREAK
			cout << "cntHeartBreak=" << cntHeartBreak << ",cnt2=" << p->cnt_2;
#endif
			if (cntHeartBreak != 0) {
				p->m_last = now;
				p->cnt_2 = 0;
				p->clearHeartBreakCnt();
			} else {
#if DEBUG_SC_HEART_BREAK
				cout << " dur=" << dur.count() << endl;
#endif
				if (dur.count() > (60000)) {
					p->cnt_2++;
					p->m_last = system_clock::now();
					if (p->cnt_2 == 2) {
						if (p->login2SC()) {
							p->setLoginState(true);
							p->cnt_2 = 0;
						}
					} else if (p->cnt_2 == 3) {
						notifyPPP("/tmp/fsu_2Dialer_recall", "1");// 通知断网了
						p->cnt_2 = 0;
						sleep(5);
						FSUServiceRestart();
						p->setLoginState(false);
						p->setScOnline(false);
					}
				}
			}
		}
		sleep(1);
	}
	return (void *)0;
}
bool LoginHdl::loginInit() {
	bLogin = false;
//	return true;
//	std::thread producer(loginHdl, this);
	if (pthread_create(&id1, NULL, loginHdlProc, this) < 0) {	// after initConfig(); !!!
		cout << "create login thread failed !" << endl;
		return false;
	}
#if 0
	if (pthread_create(&id2, NULL, ScOnlineChkProc, this) < 0) {	// after initConfig(); !!!
		cout << "create ScOnlineChk thread failed !" << endl;
		return false;
	}
	if (pthread_create(&id3, NULL, vpnOnlineChkProc, this) < 0) {	// after initConfig(); !!!
		cout << "create ScOnlineChk thread failed !" << endl;
		return false;
	}
#endif
	return true;
}
//void LoginHdl::tick() {
//	mtx.lock();
//	if (cntHeartBreak > 0) {
//		cout << "\n\nheart break in " << cntHeartBreak << "times\n\n" << endl;
//		cntHeartBreak = 0;
//	} else {
//#if DEBUG
//		cout << " !! no heart break in 120s !!" << endl;
//#endif
//		bLogin = false;
//	}
//	mtx.unlock();
//};
//void LoginHdl::createTimer() {
//	struct sigevent evp;
//	memset(&evp, 0, sizeof(struct sigevent)); //清零初始化
//	evp.sigev_value.sival_int = 123; //也是标识定时器的，这和timerid有什么区别？回调函数可以获得
//	evp.sigev_notify = SIGEV_THREAD; //线程通知的方式，派驻新线程
//	evp.sigev_notify_function = [](union sigval) {
//		loginHdl.tick();};
//
//	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
//		perror("fail to timer_create");
//		exit(-1);
//	}
//	struct itimerspec it;
//	it.it_interval.tv_sec = 0;	/* next value:下一次触发所需的时间*/
//	it.it_value.tv_sec = 60;	/*current value:目前距离触发时间点剩余的时间*/
//
//	if (timer_settime(timerid, 0, &it, NULL) == -1) {
//		perror("fail to timer_settime");
//		exit(-1);
//	}
//}
