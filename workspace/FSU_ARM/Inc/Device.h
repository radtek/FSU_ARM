/*
 * Device.h
 *
 *  Created on: 2016-6-16
 *      Author: lcz
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <string>
#include <mutex>
#include <vector>
#include "define.h"
#include "B_Interface.h"
#include "DevComm.h"
#include "RunStat.h"
#include "DevsSemaphores.h"
#include "ReqThresholds.h"
#include "HFAlarm.h"
#include "AlmFilter.h"

using namespace std;
using namespace BInt;

#define lll	//cout<< "[+]" << __FUNCTION__ << " ";
#define uuu	//cout<< "[-]" << __FUNCTION__ << "\n";
#define al		//cout<< "[++++]" << __FUNCTION__ << " ";
#define au		//cout<< "[----]" << __FUNCTION__ << "\n";
#define ml		//cout<< "[+]" << __FUNCTION__ << " ";
#define mu		//cout<< "[-]" << __FUNCTION__ << "\n";
#define ssl		cout<< "[++]" << __FUNCTION__ << " ";
#define ssu		cout<< "[--]" << __FUNCTION__ << "\n";
const u32 STAT_SETPOINT_OK = 0;
const u32 STAT_SETPOINT_BUSY = 1;
const u32 STAT_SETPOINT_DEV_RTN_FAIL = 2;
const u32 STAT_SETPOINT_DEV_RTN_FAIL_TRYAGAIN = 3;
const u32 STAT_SETPOINT_INVALID_DAT = 4;
const u32 STAT_SETPOINT_NO_REPONSE = 5;
const u32 STAT_SETPOINT_BAD_REPONSE = 6;

class Device;
class	DI {
	friend class Device;
	friend class DeviceManager;
	friend class AlmFilter;
public:
	DI();
	~DI();
	static string sAct;
	static string sDisact;
//	void calcSetVal(u32 idx2, u32 val) {
//		devSetVal(idx2, val);
//	}
//private:
	void init(const stInitDI & di);
//	void clearMem();
	void initUnit();
	void setUnitForNoRefAI (const string & u);
	void devSetVal(u32 idx2, u32 val);
	void devSetValHdl(const string & semaId, u32 val);
	bool getVal(u32 idx2, u8 & val);
	bool getGrpVal(vector <u8> &);
	int getSN(u32 semaIdx2_0);
	bool almReported(u32 semaIdx2_0);
	void setSN(u32 semaIdx2_0, u32 sn);
	void setVal(u32 semaIdx2_0, u32 val);
	void setAvail(u32 semaIdx2_0);	// set true;
	bool isAvail(u32 semaIdx2_0);
	bool isMemAlloc();
	void setMemAlloc();
	bool isChanged(u32 semaIdx2_0);
	void setChanged(u32 semaIdx2_0, bool val);
	void setFilter(bool newVal);
//	bool isLastActWasFiltered(u32 semaIdx2_0);
//	void setLastActWasFiltered(u32 semaIdx2_0, bool newVal);
	bool isFilter();
//	bool getAlarmChange (u32 idx2, bool & almValOut);
	bool getAlarmChange2 (u32 idx2, bool & almValOut);
	void fsuSetValStr(string strVal);	// 系统根据告警过滤情况来设置
	bool isFsuCtrl();
	bool setThreshold(string thrd);
	string getThreshold();
	void setDIFilterById(u32 idx2, bool);
	void fsuSetVal(/*const string & devFullId,*/ u32 semaIdx2_0, bool bRevers, bool bAlm);
	void fsuChkReverse(/*const string & devFullId,*/u32 idx2, float ref);
	void fsuCalcReverse(/*const string & devFullId,*/ u32 semaIdx2_0, bool rslt, float ref);
	void devChkReverse(u32 idx2);
	void check(const string & devFullId);
	void updateGrpSize(u32 size);
	//---------------------------------------------------
	string id;	// 1开头
	string strThreshold;
	u32 delay;
	u32 level;	// 0, or alarm level 1-4
	float threshold;
	float backlash;
//	string logic;
	int iLogic;
	string refAIId;
	bool filter;
	// --------- group ---------
	bool bGrp;
	bool bMalloc;
	u32 grpNum;
	bool * grpAvail;	// 有数据之后才可用
	u32	* grpCnt;	// 告警计数
	u8 * grpDevVal;	// 设备端获取到的数据
	u8 * grpFsuVal;	// Fsu上次数据
	float * grpAiVal;	// 告警时上报的相关模拟量值
	bool * grpChanged;
//	bool * grpLastActWasFiltered;
	HFAlarm * grpHfAlm;	// 高频告警处理
	bool * grpIsHighFreqAlm;
	u32	*grpSN;
	//----------------------------------
	string name;
	string almOnDesc;
	string almOffDesc;
	string unit;
	//----------------------------------------
	std::mutex mtx_chg;
	std::mutex mtx_mem;
	std::mutex mtx;
	std::mutex mtx_threshold;
	std::mutex mtx_filter;
	std::mutex mtx_avail;
	std::mutex mtx_sn;
	Device * upDev = NULL;
};
class	AI {
	friend class Device;
	friend class DeviceManager;
	friend class AlmFilter;
	AI();
	~AI();
	void init(const stInitAI & ai);
	bool isMemAlloc();
	void setMemAlloc();
	void setAvail(u32 semaIdx2_0);	// set true;
	bool isAvail(u32 semaIdx2_0);
	bool getValStr(u32 semaIdx2_0, string & out);
	bool getGrpValStr(vector<string> & out);
	string getUnit();
//	void setVal(string fullSemId, string strVal);
	void setValStr(u32 semaIdx2_0, string strVal);
	bool setThreshold(string absThrd, string relThrd);
	void getThreshold(string & absThrd, string & relThrd);
	void makeSemaphore(vector<stSemaphore_l>  & out);
	void updateGrpSize(u32 size);
	//------------------------------------------------
	string name;
	string id;// 1开头
	string unit;
	string strLastMeasuredVal;
	string strMeasureVal;
	string strThreshold;
	float threshold;
	u32 thresholdType; // 0: no use, 1: abs, 2: rel
//	string * lastGrpDat;
	string * grpDat;
	bool * grpAvail;	// 有数据之后才可用
	bool bGrp;
	u32 grpNum;
//	string grpRefId;
	bool bMalloc;
	// ----
	std::mutex mtx_mem;
	std::mutex mtx_avail;
	std::mutex mtx_ai;
};
class	DO {
	friend class Device;
	void init(const stInitDO & sema);
	void updateGrpSize(u32 size);
	string id;// 1开头
	bool bGrp;
	u32 grpNum;
	std::mutex mtx;
};
class	AO {
	friend class Device;
	friend class DeviceManager;
	~AO();
	void init(const stInitAO & sema);
	bool isMemAlloc();
	void setMemAlloc();
	void setAvail(u32 semaIdx2_0);	// set true;
	bool isAvail(u32 semaIdx2_0);
//	string getValStr();		//不支持组数据
	void updateGrpSize(u32 v);
	bool getValStr(u32 semaIdx2_0, string & out);
	void setValStr(u32 semaIdx2_0, string strVal);
	string id;// 1开头
//	string devVal;
//	string fsuVal;
	string * grpDat = NULL;
	bool bGrp = false;
	bool * grpAvail = NULL;	// 有数据之后才可用
	u32 grpNum = 0;
	bool bMalloc = false;
	std::mutex mtx_mem;
	std::mutex mtx_avail;
	std::mutex mtx;
//	pthread_mutex_t   mutex_lock;
};
struct devGrpSizeDef {
	string id;
	string size;
};
struct devSemaDef {
	u32 typeIdx;
	u32 maxSema[4];
//	u32 grpSize_nxx;
};
struct cmdItm {
	static const u32 MAX_ERR = 3;
	static const u32 RETRY = 1;
	string cmdId;	// DO, AO
	string val;
	u32 cntErr;		// 设置失败次数
	u32 stat;
};
class DeviceManager;
//-------------------------------------Device------------------------------------------
struct stGrpSizeItm {
	stGrpSizeItm(string id, u32 t, u32 i) : gId(id), ty(t), idx1(i) {
#if DEBUG_INIT_DAT
		cout << "stGrpSizeItim(id_n(" << id << "), ty(" << t << "),idx1(" << i << ")" << endl;
#endif
		grpSize = 0;
	}
	string gId;	// nxx or 8 char fullId !
	u32 ty;	// 0:di, 1:ai
	u32 idx1 ;
	u32 grpSize;
};
class DevComm;
class DevInit {
	friend class Device;
	void initADIO(const string & devName);
	void initGrpDat(const string & devName);
	void initThreshold(const string & devId);
	void initMaxMem(/*u32 devType*/);
	void init0(const string & devId, const string & devName);
	void setSemaAvailable(const string &id);
	//----
	u32 maxSema[4];
	vector <stInit4ADIO>  vecInitlist;
	vector <stInitDI>  vecInitDI;
	vector <pair<string,string>> vvDIsName;
	vector <stInitAI>  vecInitAI;
	vector <pair<string,string>> vvAIsName;
	vector <stInitDO>  vecInitDO;
	vector <stInitAO>  vecInitAO;
	vector <stInitGRP>  vecInitGRP;
};
template <int ASize, int DSize>
struct extDat {
	void putAIValStr(u32 idx, string s) {
		if (idx >= ASize)
			return;
		mtx.lock();
		ai_ch[idx] = s;
		mtx.unlock();
	}
	string getAIStr(u32 idx) {
		if (idx >= ASize)
			return "";
		string rtn;
		mtx.lock();
		rtn = ai_ch[idx];
		mtx.unlock();
		return rtn;
	}
	bool getAIVal(u32 idx, float & out) {
		string v = getAIStr(idx);
		if (v.empty())
			return false;
		out = out = atof(v.c_str());
	}
	void putDIVal(u32 idx, string s) {
		if (idx >= DSize)
			return;
		int v = atoi(s.c_str()) ? 1 : 0;
		mtx.lock();
		di_ch[idx] = v;
		mtx.unlock();
	}
	bool getDIVal(u32 idx, u8 & out) {
		if (idx >= DSize)
			return false;
		mtx.lock();
		out = di_ch[idx];
		mtx.unlock();
		return true;
	}
	string ai_ch[ASize];
	u8 di_ch[DSize];
	std::mutex mtx;
};
class Device {
	friend class DeviceManager;
	friend class DevComm;
	friend class AlmFilter;
public:
	Device() {
//		pNotFixGrpNum = NULL;
//		maxNotFixGrpNum = 0;
	}
//	Device(const Device & per) {
//		cout << "Device copy." << endl;
//		this->id = per.id;
//	}
	~Device();
	bool isCmdIn(string &id, string &val);
	bool getAIValById(const string & sid, float & out);
	bool getAOValById(const string & sid, float & out);
private:
	const string & getSemaName(const string & semaId);
	string getShortId();
	// ------------- init ---------------------------
//	void init_ADIO();
	void init(const string &fullId, const string & devName);
	void clearMem();

	void initDIsUnit();
	void inInitList(const string & sId, string & initVal);
	void addDISemaphore(stInitDI & sema);
	void addAISemaphore(stInitAI & sema);
	void addDOSemaphore(stInitDO & sema);
	void addAOSemaphore(stInitAO & sema);
	void initNxx();
	void initGrpSize();
// ----------------------------------------------
	DI * getDI(u32 semaIdx1_1);
	DI * getDI(string shortId);
	AI * getAI(u32 semaIdx1_1);
	DO * getDO(u32 semaIdx1_1);
	AO * getAO(u32 semaIdx1_1);
	// ----------------------------------------------
	void setCommPort(DevComm * p) {pComm  =  p;}
//	void setVal(string fullId, string val);
	void upDateSemaphoes(vector <stIdVal> & in);
	void getSemaphoes_current(vector <stSemaphore_l> & inOut);
	void getSemaphoes_history(vector <string> & inIds, vector <stSemaphore> & out
			,string startTime, string endTime);
	void upDateHistoryDat();
	void setPoint(const vector <stSemaphore_l> & in);
	void setSetPointResult(string id, u32 result);
	bool isTaskEnd();
	void getSetPointResult(vector <cmdItm> & out);
	void getThreshold(vector <stThreshold> & inOut);
	void setThreshold(const vector <stThreshold> & in,
			setThresholdReponseOfDevice & result);
	void checkDIs();
	void clearAlarm();
	bool getAlm(vector <BInt::stAlarmDB> &);

	void setDIFilterById(u32 type, bool bFilter);

	bool getDIStatById(u32 semaIdx1_1, u32 semaIdx2_0, u8 & out);
	bool getDIsStatById(u32 semaIdx1_1, vector<u8> & out);
	void setDIStatById(u32 semaIdx1_1, u32 semaIdx2_0, u32 stat);
	void setDIStatById(const string &sid, u32 stat);
	bool getAIValById(u32 idx, u32 idx2, float & out);
	bool getAOValById(u32 idx, u32 idx2, float & out);
	void setAIValById(const string & sid, float val);
	void setAIValStrById(const string & sid, string v);
	void getAIUnitById(u32 idx, string &u);
	string getFullId()  { return fullId; }
	void getAllIds(vector<string> & ids);
	void initAddNxxItm(string xx, string semaId);
	void updateNxxGrpSize(string id, u32 xx);
	void initAddGrpSizeItm(string gId, string semaId);
	void updateGrpSize(string id, u32 gs);
	bool isGrpSizeId(string id);
	bool isGrpSema(string id);
	void DO_Ctrl(const string & id, const string & val);
	void devSetDIVal(const string &semId, u32 val);
	bool isReady() {
		bool rtn = false;
		mtx_ready.lock();
		rtn = ready;
		mtx_ready.unlock();
		return rtn;
	}
	void setReady(bool b) {
		RLock(mtx_ready);
		ready = b;
	}
public:
	string devTypeId;
	string fullId;
private:
	DI** pDIs = NULL;
	AI** pAIs = NULL;
	DO** pDOs = NULL;
	AO** pAOs = NULL;
	DevInit * pInitDat = NULL;
	bool ready = false;


	//	u32 *pNotFixGrpNum; 	// 非固定大小组数据数组 //n1,n2.....,保存组数据的尺寸的引用的非标准id，和标准字典里有的id
//	u32 maxNotFixGrpNum;// 非固定大小组数据个数
	vector<pair<string, u32>> vecGrpSize_nxx;

	DevComm *pComm = NULL;
	bool datFromComm = false;

	vector <cmdItm> cmds;
	vector <stGrpSizeItm> vecNxxGrpSize;
	vector <stGrpSizeItm> vecGrpSize;
	pthread_mutex_t   mutex_lock_cmdList;
	std::mutex mtx_ready;	// 数据准备好


};
//----------------------------------------DeviceManager-------------------------------
struct battRelDevs {
	string battDevId;
	Device * pDev_06;
	Device * pDev_18;
	Device * pDev_07;
//	u32 idx_1stHalfVolt;
//	u32 idx_2ndHalfVolt;
	static constexpr auto id_1stHalfVolt = "18103001";
	static constexpr auto id_2ndHalfVolt = "18104001";
};
struct cameraRelDevs {
	static constexpr auto id_Infr 		= "18003001";
	static constexpr auto id_IllegalEntry = "17001001";
//	static constexpr auto id_MagneticDoor = "17101001";
	static constexpr auto id_doorMagnetic 	= "17005001";
	static constexpr auto id_doorLock 		= "17006001";

	bool bLastStatLight = false;
	bool bLastStatInfra = false;
	bool bLastStatDM = false;
	bool bLastStatDL = false;
	bool bLastStatIllegalEntry = false;

	string cameraDevId;
	Device * pDev_18;
	Device * pDev_17;
	DI * pDI_Infr 			= NULL;
//	AI * pAI_MagneticDoor 	= NULL;
//	,,,17005001,门磁开关状态,,,,
//	,,,17006001,门锁开关状态,,,,
	DI * pDI_doorMagnetic = NULL;
	DI * pDI_doorLock = NULL;
	DI * pDI_IllegalEntry 	= NULL;
};
class DeviceManager{
	friend class AlmFilter;
public:
	DeviceManager();
	~DeviceManager();
	bool init_local_device();
	bool run();
//	void initDeviceSema();
//	void initGrpInfo();
//	void initNxx();
//	void initGrpSize();
//	bool initDevRel();	// 关联设备
//	bool initRelDevs();
	bool initRelSemas();
	bool initPort();
	bool initDev18s();
	bool initPort_Dev18s(const portConfigItm & cfgItm);
	bool initDevComm(DevComm * p,
			const portConfigItm & cfgItm);
	bool init_alarm_filter();
	//void upDateADeviceSemaphoes(string fullDevId, vector <stIdVal> & in);
	string addr2DeviceShortId(u8 addr);
	string changeDeviceFullId(const string &fullDevId);
	string changeDeviceFullId_r(const string &fullDevId, const string & semaId);
	string changeDeviceShortId(const string &shortDevId, const string & semaId);
	string changeDeviceShortId_r(const string &shortDevId, const string & semaId);
	void semasCopy();
	void getAllDevices(vector<string> & ids);
	void getTheDeviceAllIds(string fullDevId, vector<string> & ids);
	void getTheDeviceSemaphoes_current(const string &fullDevId, vector <stSemaphore_l> & inOut);
	void getTheDeviceSemaphoes_history(const string &fullDevId,
			vector <string> & inIds, vector <stSemaphore> & out,
			string startTime, string endTime);
	void upDateHistoryDat();	// update history data in time.
	void setPoint(Devs_Semaphores & in, vector <setPointReponseOfDevice> &devResult);
	void getTheDeviceThreshold(string fullDevId, vector <stThreshold> & inOut);
	void setThreshold(ReqThresholds & in,
			vector <setThresholdReponseOfDevice> &devResult);
	void DI_CheckLoop();
	bool getReportAlms(vector<stDevAlmDB> &v);//vector <BInt::stAlarmDB> &);
	void exit();
	//bool getAIValById(string SemaId, float & f);	// 8位id
	void getAIUnitById(string &fullId, string &u);
	void copyAIVal(Device * sd, const string &s, Device * od, const string &o);
//	void chkCamerasLogic();
//	void chkCameraLogic(cameraRelDevs & );
	void filterAlarm();
	Device * findDevice(string fullDevId);
	static void * setPointProc(void *);
//	void setTheDevCmdResult(vector<cmdItm> & result);
	void DO_LightCtrl(Device * pDev_18, bool OnOff);
	void setCommStat();
	void checkGathCommStat(unsigned char bits);
//	const string & getRelDevId(const string & in, const string & refType);
	void getDevsIdsByType(const string & devType, vector<string> &vec);
	void syncOldAlms(vector<pair<string, string>> actAlms);
	void battsMeasDevSignalHdl();
	void changeTheDevice(Device * pDev, const string & fullDevId, string & newModel);
	void setNewDevice(Device * pDev, const string & fullDevId, string & newModel);
	void setEmptyDevice(Device * pDev, const string & fullDevId);
	void doorMagneticLogic();
	void cameraLogic();
	void quitNow() {
		runStat.setRun(false);
	}
	void startNow() {
		runStat.setRun(true);
	}
private:
//	bool getDIStatById(string fullSemaId);
//	bool DI_IsAct(string sId);
//	bool camerasSignalInit();
//	bool cameraSignalInit(const string & devId_env);
	bool cameraSignalInit();
	void chkCameraLogic(cameraRelDevs & st);
//	bool battsMeasDevSignalInit();
//	bool battMeasDevSignalInit(const string & devId_batt);
	void battMeasDevSignalHdl(Device * p);
//	bool makeDevice(vector<BInt::devIdCode> &devList);
public:
	bool bRunning = false;
	extDat<8, 8> allExtDat[8];
	RunStat runStat;
private:
	vector<Device*>** pDevs = NULL;
	vector<AlmFilter*> filters;

	vector<Device *> vecBattDevs;
	vector<DevComm *> vecDevComm;
//	vector<cameraRelDevs> vecCameraRelDevs;

	u32 maxDev = 100;	// maxDevIdx + 1 !!!
//	devSemaDef * pDevSemaDefs;

	// ******** 告警过滤相关 ********
	Device * pDev_06 = NULL;	// 开关电源 for batt dev 07
//	Device * pDev_07 = NULL;	// batt
//	Device * pDev_17;	// 门禁
//	Device * pDev_18;	// 机房环境
	Device * pDev_17 = NULL;	// 通信中断
	Device * pDev_19 = NULL;	// 通信中断
//	DI *	pDI_IllegalEntry;
//	AI *	pAI_MagneticDoor;
//	DI *	pDI_Infr;

//	vector<pair<string, string>> vecDevRelPair;

	Devs_Semaphores setPointDat;
	vector <setPointReponseOfDevice> setPointResult;
	pthread_mutex_t   mutex_lock_setPoint_busyFlag;
//	std::mutex mtx_running;
	pthread_t tid_loop;
};

#endif /* DEVICE_H_ */
