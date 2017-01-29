/*
 * Gatherer.h
 *
 *  Created on: 2016年9月20日
 *      Author: lcz
 */

#ifndef GATHERER_H_
#define GATHERER_H_

#include <mutex>
#include <condition_variable>

#include "DevComm.h"
#include "MultiThrdSerial.h"

class Gatherer/* : public SuperDevComm*/ {
public:
	Gatherer();
	virtual ~Gatherer();
	bool init();
//	void commErrClear(int idx);
	void set(unsigned char);
	unsigned char get();

	MultiThrdSerial *comPort485 = NULL;

	void clearCommStat() {
		mtx_exStat.lock();
		unsigned char mask = ~(1 << lastAddr);
		extBoardCommStatByte &= mask;
		mtx_exStat.unlock();
	}
	void setCommStat() {
		mtx_exStat.lock();
		unsigned char mask = (1 << lastAddr);
		extBoardCommStatByte |= mask;
		mtx_exStat.unlock();
	}
	bool chkDatBack();
	bool isOnRcv();
	void setOnRcv(bool b);
	void clear() {
		_pos = 0;
		comPort485->setOwner((void*)0);
		setOnRcv(false);
		comPort485->clear();
	}
private:
	unsigned char _buf[16];
	unsigned _pos = 0;
//	unsigned char buf[4096];
//	unsigned int bufSize = 4096;
//	unsigned int savePos = -1;
public:
	unsigned char extBoardCommStatByte;
	bool use485Port = false;
	bool installStat[8];
	unsigned int idx = 0;
	unsigned char lastAddr;
	char lastSendByte;
	bool m_onRcv = false;
	std::condition_variable cv;
	std::mutex	mtx_cond_rcv;
	std::mutex mtx_exStat;
	std::mutex mtx_onRcv;
};

#endif /* GATHERER_H_ */
