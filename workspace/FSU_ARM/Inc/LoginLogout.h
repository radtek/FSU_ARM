/*
 * LoginLogout.h
 *
 *  Created on: 2016-8-13
 *      Author: vmuser
 */

#ifndef LOGINLOGOUT_H_
#define LOGINLOGOUT_H_

#include <mutex>
#include <signal.h>
#include <chrono>

using namespace std;
using namespace chrono;

class LoginHdl {
public:
	bool loginInit();
	bool login2SC();
	bool logout();
	bool isScOnline() {
		bool rtn = false;
		mtx_scOnLine.lock();
		rtn = bScOnLine;
		mtx_scOnLine.unlock();
		return rtn;
	}
	void setScOnline(bool b) {
		std::lock_guard<std::mutex> lock(mtx_scOnLine);
		bScOnLine = b;
	}
	bool isVpnOnline() {
		bool rtn = false;
		mtx_vpnOnLine.lock();
		rtn = bVpnOnLine;
		mtx_vpnOnLine.unlock();
		return rtn;
	}
	void setVpnOnline(bool b) {
		std::lock_guard<std::mutex> lock(mtx_vpnOnLine);
		bVpnOnLine = b;
	}
	bool isLogin() {
		bool rtn = false;
		mtx_login.lock();
		rtn = bLogin;
		mtx_login.unlock();
		return rtn;
	}
	void setLoginState(bool );

//	void tick();
//	void createTimer();
	void heartBreak() {
		std::lock_guard < std::mutex > lock(mtx_HB);
		cntHeartBreak++;
		mtx_scOnLine.lock();
		bScOnLine = true;
		mtx_scOnLine.unlock();
	}
	void clearHeartBreakCnt() {
		std::lock_guard < std::mutex > lock(mtx_HB);
		cntHeartBreak = 0;
	}
	unsigned int getHeartBreakCnt() {
		unsigned int rtn;
		mtx_HB.lock();
		rtn = cntHeartBreak;
		mtx_HB.unlock();
		return rtn;
	}
public:
	unsigned int cnt_2 = 0;
	bool bScOnLine = false;
	bool bVpnOnLine = false;
	system_clock::time_point m_last = system_clock::now();
private:
//	timer_t timerid;
	std::mutex mtx_login;
	std::mutex mtx_scOnLine;
	std::mutex mtx_vpnOnLine;
	std::mutex mtx_HB;
	unsigned int cntHeartBreak;
	bool	bLogin;
	pthread_t id1, id2, id3;
};
#endif /* LOGINLOGOUT_H_ */
