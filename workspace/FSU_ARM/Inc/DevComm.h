/*
 * DevComm.h
 *
 *  Created on: 2016-7-19
 *      Author: lcz
 */

#ifndef DEVCOMM_H_
#define DEVCOMM_H_

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include "define.h"
#include "B_Interface.h"
#include "RunStat.h"
#include "Device.h"
#include "MultiThrdSerial.h"
#include "PrtclDat.h"


using namespace std;
using namespace chrono;
using namespace BInt;
#define scl	//	cout<< "[++]" << __FUNCTION__ << " ";
#define scu	//	cout<< "[--]" << __FUNCTION__ << "\n";

const s32 T_GRP_BRACKET_LEFT 	= -1;	// left bracket
const s32 T_GRP_BRACKET_NOT 	= 0;	//
const s32 T_GRP_BRACKET_RIGTH 	= 1;	// right bracket
//struct lastCmdItm {
//	u32 type;
//	u32 idx;
//};
typedef bool (*PFN_getOnePak) (const unsigned char * comBuf, unsigned int len,
		unsigned char * m_frmBuf, unsigned int & m_frmBufPos, unsigned int & remain);
typedef bool (*PFN_isCRCOk) (const unsigned char * s, unsigned int len);
typedef void (*PFN_addCRC) (unsigned char * s, unsigned int & len);
typedef bool (*PFN_isCmdOk) (const unsigned char * in, unsigned int inLen,
		const unsigned char * out, unsigned int outLen);
typedef string (*PFN_getValStr) (const unsigned char *s, unsigned int len, int mul, bool sign);
typedef string (*PFN_setValStr) (float f, unsigned int len, int mul);

#define PFN(y) PFN_##y y
#define LOAD_PFN(x) x = (PFN_##x) dlsym(hDynLib, #x); \
		if ((error = dlerror()) != NULL) {\
			cout << "load dynfunc error (" << error << ")" << endl;\
			return  false;\
		}
static const u32 COMM_ERR_MAX_NUM = 3;
static const u32 COMM_TRAP_ERR_MAX_NUM = 3;

class Device;
//class MultiThrdSerial;
//struct MyCommDat {
//	int pos;
//	bool eod;
//	u8 in[4096];
//};
struct stDevDefCfgItm {
	string productModelIdx;
	string productModel_name;
	string devType;
	string format;
	string ver;
	u32 bufSize;
	u32 baudrate;
	u8 databit;
	char parity;
	string stopbit;
	string addr;
	u32 maxdelay;
};
struct protocolFormat {
	string	type;
	string	soi;
	string	eoi;
	string	code;
};
struct ProtocolParse {
	ProtocolParse(const protocolFormat & fmt);
	~ProtocolParse();
	bool LoadPFN();
	bool valid() { return hDynLib; }
	void * hDynLib;
	PFN(getOnePak);
	PFN(isCRCOk);
	PFN(addCRC);
	PFN(isCmdOk);
	PFN(getValStr);
	PFN(setValStr);
	bool initProtocolFormat(const vector<string> & cfg);
	string so;
	protocolFormat fmt;
};
//class SuperDevComm {
//	virtual void devPakIn(const unsigned char * comBuf, u32 size) = 0;
//};
class DevComm/* : public SuperDevComm*/ {
public:
	struct scanDat {
		const stDevDefCfgItm * defCfg;	// model, head info, 9600,N,8,1
		const ProtocolParse * ppp;		// PFN ... , 1383.3 in fmt
	};
public:
	DevComm();
//private:
	~DevComm();
//	bool init(const stDevInitCommCfg &);
	bool initScanTbl(const portScanTblItm & in);
//	void initDefCfg(const stDevDefCfgItm & c);
	void startScan();
	bool setDevice(string devId);
	bool initPortDat();
	bool initWorkThread();
	bool init(const string & tty, const string & port, const string & devId);
	bool initPort();
	void init_buf_head();
	bool loop();
//	void devPakIn(unsigned char * comBuf, u32 len);
	u32 parseThePak_bit(unsigned char * const buf, s32 start, s32 end);
	void ascStrOneBitHdl(string bxx, char h, char l);
	void parseThePak2(unsigned char * const buf, u32 size);
	void parseScanRtnPak(unsigned char * const buf, u32 size);
	void setPrtcl(PrtclDat * p) {
		prtclDat = p;
	}
	void setPrtcl();
	bool parseThePak(unsigned char * const buf, u32 size);
	bool packHdl();
	void noPackResponseHdl();
	void badPackResponseHdl();
	void commEvent(bool b);
	bool isCommErr();
	bool isOnRcv();
	void setOnRcv(bool b);
	bool isOnScan();
	void setOnScan(bool b);
	bool isRunning() {
//		bool rtn = false;
//		mtx_run.lock();
//		rtn = bRunning;
//		mtx_run.unlock();
//		return rtn;
		return runStat.isRunning();
	}
	void startThread() {
//		mtx_run.lock();
//		bRunning = true;
//		mtx_run.unlock();
		runStat.setRun(true);
	}
	void quitThreadProc() {
//		mtx_run.lock();
//		if (!isInDuty() && !isInPackChk())
//			bRunning = false;
//		mtx_run.unlock();
		runStat.setRun(false);
	}
//	bool isInDuty() {
//		bool rtn = false;
//		mtx_dutyStat.lock();
//		rtn = bInDutyStat;
//		mtx_dutyStat.unlock();
//		return rtn;
//	}
//	void setInDutyStat(bool TorF) {
//		mtx_packChkStat.lock();
//		bInDutyStat = TorF;
//		mtx_packChkStat.unlock();
//	}
//	bool isInPackChk() {
//		bool rtn = false;
//		mtx_packChkStat.lock();
//		rtn = bInPackChkStat;
//		mtx_packChkStat.unlock();
//		return rtn;
//	}
//	void setPackChkSta(bool TorF) {
//		mtx_packChkStat.lock();
//		bInPackChkStat = TorF;
//		mtx_packChkStat.unlock();
//	}

private:
	bool onScan = true;
	u32  scanSize = 0;
	bool scanMode1_devConn = false;
	bool scanMode1_devConn_1Loop = false;
	u32 nTTyType = 0;
	int devType = -1;
	string strDevId = "";
	string comPathName = "";
	string modelIdx = "";
	pthread_t tid_duty = 0;
	pthread_t tid_packChk = 0;
public:
	u32  scanMode = 0;
	string ttyType = "";
	string ttyPort = "";
	RunStat runStat;
//	bool connBy485() { return comPort->connBy485(); }
//	bool directConn() { return comPort->directConn(); }
inline	void clear() { // give up last req, new req.
		m_gotHead = false;
		m_lastPos = 0;
		m_bufPos = 0;
		comPort->setOwner((void*)0);
		setOnRcv(false);
		comPort->clear();
	}
	bool myTurn() {	return comPort->isMyFD(m_subFD); }
	bool configPort();
	void setCurrDevDat(scanDat * p) {
		currDevDat = p;
	}
private:
	string bitSetCtrl(const P_SetCtrl &, string cmdId, string cmdVal);
	u32 output(unsigned char * pBuf, u32 len);
	void makePak(string szIn, unsigned char * szOut, u32 &len);
	void makeDutyReqPak(unsigned char * szOut, u32 &len);
	void makeScanPak(unsigned char * szOut, u32 &len);
	void setScanMode();
	void makeCmdPak(string & cmdInf, unsigned char * szOut, u32 &len);
	void nextGet();
	bool OpenCommPort();	//
	bool initProtocolParseDat(string & file);	// init protocol parse data
	bool initProtocolFormat(string & file);		// init protocol format
	void prepareDevice(u32 idxInScanTbl);
public:
	Device * pDev = NULL;
	MultiThrdSerial *comPort = NULL;
	int m_subFD = -1;

//	bool bRunning = false;
//	bool bInDutyStat = false;
//	bool bInPackChkStat = false;
	bool onGet = true;
	bool bCommErr = true;
	u32 getIdx = 0;
	u32 lastGetIdx = 0;
	u32 scanIdx = 0;
	u32 lastScanIdx = 0;
	string cmdId;
	string cmdVal;
	bool scanable = false;

	u8 *m_buf = NULL;	// all buffer
	u32 m_bufPos = 0;	// datBuf write position
	unsigned char oBuf[256];
	size_t outLen = 0;
//	pthread_cond_t cond_rcv;
//	pthread_mutex_t mtx_cond;
	std::condition_variable cv;
	std::mutex	mtx_cond_rcv;
	//pthread_mutex_t   mutex_inbuf_lock;
	std::mutex mtx_inbuf;
	std::mutex mtx_comm;
	std::mutex mtx_onRcv;
	std::mutex mtx_onScan;
	std::mutex mtx_scanIdx;
//	std::mutex mtx_run;
	std::mutex mtx_dutyStat;
	std::mutex mtx_packChkStat;
	bool bExDev = false;
	bool b485 = false;
	vector <scanDat> scanTbl;
	u32 *sortedTblOfMode1 = NULL;
private:
	u32 commErrCounter = COMM_ERR_MAX_NUM;
	u32 commTrapCounter = 0;
	u32 multiByte = 0;

	string pakHead;
	PrtclDat * prtclDat = NULL;
	const portScanTblItm * scanInitTbl = NULL;
//	const stDevDefCfgItm * defCfg = NULL;
//	ProtocolParse * ppp;
	const scanDat * currDevDat = NULL;
	u32 cntDuty = 0;
	u32 cntDutyScan = 0;
//	bool exTimeout = false;
public:
	unsigned long long m_lastTime = 0;
	system_clock::time_point m_last = system_clock::now();
	bool m_onRcv = false;
	bool m_gotHead = false;
	unsigned int m_lastPos = 0;
//	std::mutex mtx_port_io;
};

bool createPorts();

#endif /* DEVCOMM_H_ */
