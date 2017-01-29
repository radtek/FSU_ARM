/*
 * Gatherer.cpp
 *
 *  Created on: 2016年9月20日
 *      Author: lcz
 */
#include <myComm.h>
#include <unistd.h>
#include "Device.h"
#include "Gatherer.h"
#include "AppConfig.h"

Gatherer::Gatherer() {
	// TODO Auto-generated constructor stub
	extBoardCommStatByte = 0;
	lastSendByte = -1;
	lastAddr = 8;	// so next goto 0 !
}

Gatherer::~Gatherer() {
	// TODO Auto-generated destructor stub
	if (comPort485) {
		delete comPort485;
		comPort485 = NULL;
	}
}


extern GlobalDat gDat;
extern DeviceManager DevMgr;
extern std::mutex mtx_can485_wrt;
//void * HeartPakChkProc(void * param) {
//	Gatherer * p = (Gatherer *) param;
//	while(p->extBoardCommStatByte > 0) {
//		DevMgr.chkGathCommStat(p->get());
//		p->set(0);	// all set 0
//		usleep(2);
//	}
//	return (void *)0;
//}
bool Gatherer::chkDatBack() {
	bool rtn = false;
//	cout << "[c]";
	if (!comPort485->isEmpty()) {
//		cout << "heart break check\n";
		u32 pos = comPort485->pos();
//		cout << "check heart break, pos =" << pos << "\n";
		if (pos == 4) {
			printHex(comPort485->m_portBuf, 4);
			unsigned char tmp[4];
			comPort485->getBuf4Byte(tmp);
//			cout << "lastSendByte=" << (u32)lastSendByte << endl;
			if (my_transform(485, comPort485->getTTY(), _buf, &_pos, tmp, 4) != 0) {
				if (_buf[0] == lastSendByte) {
//					SHOW_TIME("notify back");
					cv.notify_one();
					rtn = true;
				}
//				else
//					printf("wrong buf dat -> %02x\n", _buf[0]);
			}
//			else
//				SHOW_TIME("heart check bad !\n");
		}
	}
	return rtn;
}
bool Gatherer::isOnRcv() {
	bool rtn = false;
	mtx_onRcv.lock();
//	cout << "[ + Gatherer isOnRcv()?";
	rtn = m_onRcv;
//	cout << (rtn ? (u32)1 : (u32)0) ;
//	cout << " - ]\n";
	mtx_onRcv.unlock();
	return rtn;
}
void Gatherer::setOnRcv(bool b) {
	mtx_onRcv.lock();
	comPort485->setOwner(this);
//	cout << "[ + Gatherer setOnRcv";
	m_onRcv = b;
//	cout << " - ]\n";
	mtx_onRcv.unlock();
}
void * myHeartLoop(void * param) {
	SET_THRD_NAME();
	Gatherer * p = (Gatherer *) param;
	while(gDat.gatherersInstallDat8 > 0) {
		threadDebug
		if (p->use485Port) {
			while(p->isOnRcv()) {
//				cout << "p:" << p << "," << (p->comPort485->owner) << endl;
				if(!p->comPort485->me(p)) {
					continue;
				}
//				cout << "[*] device-" << p << "- Gatherer" << endl;
				p->chkDatBack();
				usleep(10000);
			}
			usleep(10000);
		} else {
			DevMgr.checkGathCommStat(p->get());
			p->set(0);	// all set 0
			sleep(2);
		}
	}
	return (void *)0;
}
static unsigned char addr[8] = {0,1,2,3,4,5,6,7};
//			{0,0x10,0x20,0x30,0x40,0x50,0x60,0x70};

void * GathererHeartLoop(void * param) {
	SET_THRD_NAME();
	Gatherer * p = (Gatherer *) param;
	while(gDat.gatherersInstallDat8 > 0) {
		threadDebug
		if (p->use485Port) {
			usleep(250000);
			p->mtx_exStat.lock();
			p->idx = (p->idx + 1) % 8;
//			cout << "p->idx = " << p->idx << endl;
			if (p->idx == 0) {
				u8 st = p-> extBoardCommStatByte;		// no need lock here !
#if DEBUG_485_HEART
				cout << " break state = " << (u32) st << endl;
#endif
				DevMgr.checkGathCommStat(st);
			}
			if (p->installStat[p->idx]) {
				p->lastAddr  = p->idx;
				p->lastSendByte = addr[p->lastAddr];
			} else
				p->lastAddr = 255;
			p->mtx_exStat.unlock();
			if (p->lastAddr != 255) {
				p->comPort485->setFD(485);
//				cout << "lastAddr=" << p->lastAddr << endl;
				p->comPort485->writeSerial(addr + p->lastAddr, 1);
				p->setOnRcv(true);
#if DEBUG_485_HEART
				SHOW_TIME("heart break out !\n");
#endif
				std::unique_lock <std::mutex> lk (p->mtx_cond_rcv);
				std::cv_status ret = p->cv.wait_for(lk, std::chrono::milliseconds(1500));
				p->clear();
//				cout << "time out clear ????\n";
				if(ret == std::cv_status::timeout) {
#if DEBUG
				SHOW_TIME( "<- heart break timeout ! \n");
#endif
					p->clearCommStat();
				} else {
					p->setCommStat();
				}
			}
		} else {
			unsigned char addr = heartLoop();
			if (addr != 0) {
				unsigned char old = p->get();
				old = old | (1 << (addr - 1));
				p->set(old);
			}
			usleep(250000);
		}
	}
	return (void *)0;
}
bool Gatherer::init() {
	set(gDat.gatherersInstallDat8);
	for (int i = 0; i < 8; ++i)
		installStat[i] = (((1 << i) & extBoardCommStatByte) > 0);
	int ttyType = *(gDat.cfgs[GIDX_FSUCOMMTYPE].c_str()) - '0';
	use485Port = (ttyType == 2);
//	comPort = NULL;
	if (gDat.gatherersInstallDat8 > 0) {
#if DEBUG_GATHER_ON
		pthread_t id;
		int ret = pthread_create(&id, NULL, myHeartLoop, this);
		if (ret < 0)
			return false;
		pthread_t id2;
		ret = pthread_create(&id2, NULL, GathererHeartLoop, this);
		if (ret < 0)
			return false;
#endif
		my_init(ttyType, gDat.baudRate485);
		if (use485Port) {
			comPort485 = new MultiThrdSerial;
			const char * stop = "1";
			if ( ! comPort485->open_config("485ex",	2,
					gDat.baudRate485, 8, stop, 'N', 4096, false)) {
				comPort485->closePort();
				cout << "open ext 485 port error !!" << endl;
				return false;
			}
		}
	}

	return true;
}
//void Gatherer::commErrClear(int idx) {
//	if (idx >= 8)
//		return;
//	unsigned char mask = 1 << idx;
//	extBoardCommStatByte &= ~mask;	// clear the bit
//}
void Gatherer::set(unsigned char c) {
	mtx_exStat.lock();
	extBoardCommStatByte = c;
	mtx_exStat.unlock();
}
unsigned char Gatherer::get() {
	unsigned char rtn = 0;
	mtx_exStat.lock();
	rtn = extBoardCommStatByte;
	mtx_exStat.unlock();
	return rtn;
}

Gatherer Gath;
//void commErrClear(int idx) {
//	Gath.commErrClear(idx);
//}


