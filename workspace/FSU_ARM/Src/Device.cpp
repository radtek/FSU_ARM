/*
 * Device.cpp
 *
 *  Created on: 2016-6-16
 *      Author: lcz
 */
#include <unistd.h>
#include <thread>
#include <sstream>
#include <tuple>
#include "Device.h"
#include "AppConfig.h"
#include "B_Interface.h"
#include "AlarmHdl.h"
#include "DB.h"
#include "AutoIncNum.h"
#include "Pipe.h"
#include "DevComm.h"

using namespace BInt;
using namespace std;

DeviceManager DevMgr;
extern GlobalDat gDat;
extern DBHdl DB;
static u32 almOffDelay = 5;
DI::DI() {
	iLogic = 0;
	filter = false;
	backlash = 0;
	delay = 0;
	level = 0;
	threshold = 0;
	bMalloc = false;
	grpNum = 0;
	bGrp = false;

	grpAvail = NULL;
	grpCnt = NULL;
	grpDevVal = NULL;
	grpFsuVal = NULL;
	grpAiVal = NULL;
	grpChanged = NULL;
	grpHfAlm = NULL;
	grpIsHighFreqAlm = NULL;
	grpSN = NULL;
//	grpLastActWasFiltered = NULL;
}
DI::~DI() {
	if (!isMemAlloc())
		return;
	if (bGrp) {
		delete[] grpFsuVal;
		grpFsuVal = NULL;
		delete[] grpDevVal;
		grpDevVal = NULL;
		delete[] grpCnt;
		grpCnt = NULL;
		delete[] grpAiVal;
		grpAiVal = NULL;
		delete[] grpAvail;
		grpAvail = NULL;
		delete[] grpChanged;
		grpChanged = NULL;
		delete[] grpHfAlm;
		grpHfAlm = NULL;
		delete[] grpIsHighFreqAlm;
		grpIsHighFreqAlm = NULL;
		delete[] grpSN;
		grpSN = NULL;
	} else {
		delete grpFsuVal;
		grpFsuVal = NULL;
		delete grpDevVal;
		grpDevVal = NULL;
		delete grpCnt;
		grpCnt = NULL;
		delete grpAiVal;
		grpAiVal = NULL;
		delete grpAvail;
		grpAvail = NULL;
		delete grpChanged;
		grpChanged = NULL;
		delete grpHfAlm;
		grpHfAlm = NULL;
		delete grpIsHighFreqAlm;
		grpIsHighFreqAlm = NULL;
		delete grpSN;
		grpSN = NULL;
	}
	//删除临界区
//	pthread_mutex_destroy(&mutex_lock);
}
string DI::sAct = string("1");
string DI::sDisact = string("0");

u32 almTrans(const string & s) {
	if (s == "")
		return 0;	// not alarm !
	else if (s == "一级告警")
		return 1;
	else if (s == "二级告警")
		return  2;
	else if (s == "三级告警")
		return 3;
	else if (s == "四级告警")
		return 4;
	else
		return 0;
}
void DI::init(const stInitDI & sema) {
	id = sema.id;
	unit = sema.refAiUnit;
	strThreshold = sema.threshold_sel;
	threshold = str2Float(strThreshold);
	level = almTrans(sema.level_sel);

	if (!strThreshold.empty()) {
		const char * s;
		if (sema.logic.size() != 0) {
			s = sema.logic.c_str();
			if ((*s == 'h') || (*s == 'H'))
				iLogic = 1;
			else if ((*s == 'l') || (*s == 'L'))
				iLogic = -1;
			else {
				iLogic = 0;
//				cout << " [logic maybe lost !]" << endl;
			}
		}
	}

	if (sema.backlash.size() > 0) {
		delay = 0;
	} else {
		if (sema.delay_sel.size() > 0)
			delay = atoi(sema.delay_sel.c_str());
		else
			delay = level * 60;
#if DEBUG
		delay /= 10;
#endif
#if DEBUG_INIT_DAT
		cout << ", delay = " << delay;
#endif
	}
//cout << "****init DI:" << sema.id << ",level=" << level << ",delay="
//		<< delay << endl;

//	if (!strThreshold.empty()) {
//		if (sema.logic == "h")
//			bigLogic = true;
//		else if (sema.logic == "l")
//			bigLogic = false;
//		else
//			cout << " [logic maybe lost !]" << endl;
//	}
	refAIId = sema.refAIId;
#if	DEBUG_INIT_DAT
	cout << ", refAIId = " << refAIId << ", threshold =" << strThreshold << endl;
#endif
	backlash = atof(sema.backlash.c_str());
	name = sema.name;
	almOnDesc = sema.almOnDesc;
	almOffDesc = sema.almOffDesc;

	// group data
	bGrp = sema.bGrp;
	if (bGrp) {
		grpNum = 0;
		if (sema.bFixNum) {
			grpNum = sema.grpNum;
#if DEBUG_INIT_DI
			cout << "in DI_Init(), id = " << id << ", grpNum = " << grpNum << endl;
#endif
			grpFsuVal = new u8[grpNum];
			grpDevVal = new u8[grpNum];
			grpCnt = new u32[grpNum];
			grpAiVal = new float[grpNum];
			grpAvail = new bool[grpNum];
			grpHfAlm = new HFAlarm[grpNum];
			grpIsHighFreqAlm = new bool[grpNum];
			grpChanged = new bool[grpNum];
			grpSN = new u32[grpNum];
//			grpLastActWasFiltered = new bool[grpNum];
			for (u32 i = 0; i < grpNum; ++i) {
				grpAvail[i] = false;
				grpChanged[i] = false;
				grpCnt[i] = 0;
				grpIsHighFreqAlm[i] = false;
				grpFsuVal[i] = 0;
				grpDevVal[i] = 0;
				grpSN[i] = -1;
//				grpLastActWasFiltered[i] = false;
			}
			setMemAlloc();
		} else {
//			grpRefId = sema.gId;
//			cout << "init DI:" << id << "," << "grpRefId = " << grpRefId
//					<< endl;
			bMalloc = false;
		}
	} else {
		grpNum = 1;
		grpFsuVal = new u8;
		grpDevVal = new u8;
		grpCnt = new u32;
		grpAiVal = new float;

		grpAvail = new bool;
		*grpAvail = false;
		grpChanged = new bool;
		*grpChanged = false;
		*grpCnt = 0;
		grpHfAlm = new HFAlarm;
		grpIsHighFreqAlm = new bool;
		*grpIsHighFreqAlm = false;
		grpSN = new u32;
		*grpFsuVal = 0;
		*grpDevVal = 0;
		*grpSN = -1;
//		grpLastActWasFiltered = new bool;
//		*grpLastActWasFiltered = false;

		setMemAlloc();
	}
//	pthread_mutex_init(&mutex_lock, NULL);
}
void DI::updateGrpSize(u32 size) {
	if (!bGrp)
		return;
	if (size > grpNum) {
		mtx.lock();
		lll
		if (isMemAlloc()) {
			delete[] grpFsuVal;
			delete[] grpDevVal;
			delete[] grpCnt;
			delete[] grpAiVal;
			delete[] grpAvail;
			delete[] grpHfAlm;
			delete[] grpIsHighFreqAlm;
			delete[] grpChanged;
			delete[] grpSN;
			grpFsuVal = NULL;
			grpDevVal = NULL;
			grpCnt = NULL;
			grpAiVal = NULL;
			grpAvail = NULL;
			grpChanged = NULL;
			grpHfAlm = NULL;
			grpIsHighFreqAlm = NULL;
			grpSN = NULL;
		}

		grpNum = size;
		grpFsuVal = new u8[grpNum];
		grpDevVal = new u8[grpNum];
		grpCnt = new u32[grpNum];
		grpAiVal = new float[grpNum];
		grpAvail = new bool[grpNum];
		grpHfAlm = new HFAlarm[grpNum];
		grpIsHighFreqAlm = new bool[grpNum];
		grpChanged = new bool[grpNum];
		grpSN = new u32[grpNum];
//		grpLastActWasFiltered = new bool[grpNum];
		for (u32 i = 0; i < grpNum; ++i) {
			grpAvail[i] = false;
			grpChanged[i] = false;
			grpCnt[i] = 0;
			grpIsHighFreqAlm[i] = false;
			grpFsuVal[i] = 0;
			grpDevVal[i] = 0;
			grpSN[i] = -1;
//			grpLastActWasFiltered[i] = false;
		}
		uuu
		mtx.unlock();
		setMemAlloc();
	}
}
void DI::initUnit() {
	if (unit.empty())
		DevMgr.getAIUnitById(refAIId, unit);
#if DEBUG_INIT_DAT
	cout << "DI (" << id << ")-refAI(" << refAIId << ") ---- unit is [" << unit
	<< "]" << endl;
#endif
}
void DI::setUnitForNoRefAI (const string & u) {
	unit = u;
}
bool DI::getVal(u32 semaIdx2_0, u8 & out) {
//#if DEBUG_DAT_DETAIL
//	string sid = semaIdCalcAdd(id, semaIdx2_0);
//	cout << "\t\t\t"<< __FUNCTION__ << ":[" << sid << "]" << endl;
//#endif
	if (!isAvail(semaIdx2_0)) return false;
	if (!isMemAlloc()) return false;
#if DEBUG_DAT_DETAIL
	string sid = semaIdCalcAdd(id, semaIdx2_0);
#endif
//	if (sid == "25004001")
//		cout << "now!" << endl;
	if (!bGrp) {
		mtx.lock();
		lll
//		if (level == 0)
//			out = *grpDevVal;
//		else
			out = *grpFsuVal;
		uuu
		mtx.unlock();
	} else {
		if ((semaIdx2_0 >= 0) && (semaIdx2_0 < grpNum)) {
			mtx.lock();
			lll
//			if (level == 0)
//				out = grpDevVal[semaIdx2_0];
//			else
				out = grpFsuVal[semaIdx2_0];
			uuu
			mtx.unlock();
		}
	}
//#if DEBUG_DAT_DETAIL
//	sid = semaIdCalcAdd(id, semaIdx2_0);
//	cout << "\t\t\t"<< __FUNCTION__ << "[" << sid << "] = " << out << endl;
//#endif
	return true;
}
bool DI::getGrpVal(vector <u8> & out) {
	if (isMemAlloc()) {
		RLock(mtx);
		lll
		for (u32 i = 0; i < grpNum; ++i) {
			if (isAvail(i) )
				out.push_back(grpFsuVal[i]);
		}
		uuu
	} else
		return false;
	return true;
}
bool almJudge[8][5] = {
		{0,0,0,0,0},
		{0,0,1,0,0},
		{0,1,0,1,0},
		{0,1,1,1,0},
		{1,0,0,1,1},
		{1,0,1,0,0},
		{1,1,0,0,0},
		{1,1,1,1,0}
	};
bool DI::getAlarmChange2(u32 semaIdx2_0, bool & almValOut) {
	bool save2SC;
	u32 idx2 = (bGrp) ? semaIdx2_0 : 0;
//	bool now, last, ftr, out, rst;// currVal, reportedNow, filter stat, output 2 SC, result
	mtx.lock();
	bool c1 = (grpFsuVal[idx2] > 0);
	bool c2 = almReported(idx2);
	bool c3 = isFilter();
	for (u32 i = 0; i < 8; ++i) {
		if (   (almJudge[i][0] == c1)
			&& (almJudge[i][1] == c2)
			&& (almJudge[i][2] == c3) ) {
			save2SC = almJudge[i][3];
			almValOut = almJudge[i][4];
			break;
		}
	}
//	if (id == "06026001" && (semaIdx2_0 == 2)) {
//		cout << "now\n";
//		cout << c1 << "," << (u32)c2 << "," << (u32)c3 << "," << (u32)save2SC << "," << almValOut << endl;
//	}
	if (c1)
		grpHfAlm[idx2].setOnState();

	if (save2SC) {
		if (almValOut) {
			cout << "HF check(" << id << "," << semaIdx2_0 << "):";
			if (!grpIsHighFreqAlm[idx2]) {
				if(	(grpIsHighFreqAlm[idx2] = grpHfAlm[semaIdx2_0].addNewAndCheckActive(time(NULL)))) {
					cout << endl << "High Freq Alarm Active [" << id << "]" << endl << endl;
					grpHfAlm[idx2].setOnState();
				}
			} else {
				save2SC = false;
				grpHfAlm[idx2].setOnState();
			}
		} else {
			if (c3) {
				grpHfAlm[idx2].reset();
				grpIsHighFreqAlm[idx2] = false;
			} else {
				if (grpIsHighFreqAlm[idx2]) {// 高频告警时
					cout << "HF clear check(" << id << "," << semaIdx2_0 << "):";
					if (grpHfAlm[idx2].isDisactive()) {// 高频恢复
						grpIsHighFreqAlm[idx2] = false;
						cout << endl << "High Freq Alarm DisActive ["<< id << "]" << endl << endl;
					} else {
						save2SC = false;
					}
				}
			}
		}
	}
	mtx.unlock();

	return save2SC;
}
//bool DI::getAlarmChange(u32 semaIdx2_0, bool & almValOut) {
//	bool save2SC = false;
//	RLock(mtx);
//	lll
//
//	u32 idx2 = (bGrp) ? semaIdx2_0 : 0;
//	if (isFilter()) {
//		if (almReported(idx2)) {
//			save2SC = true;
//			almValOut = false;
//		} else
//			save2SC = false;
//	} else {
//#if 0
//		if (!bGrp && isChanged(0)) {
//			if (*grpFsuVal > 0) { //	产生
//				almValOut = true;
//				setChanged(0, false);
//				if (!*grpIsHighFreqAlm) {
//					if(	(*grpIsHighFreqAlm = grpHfAlm->addNewAndCheckActive(time(NULL))))
//						cout << endl << "{{{ High Freq Alarm Active [" << id << "] }}}" << endl << endl;
//					save2SC = true;
//				} else {
//					save2SC = false;
//					grpHfAlm->setOnState();
//				}
//			} else {				// 恢复
//				if (!almReported(0)) {
//					save2SC = false;
//					almValOut = false;
//					setChanged(0, false);
//				} else {
//					if (*grpIsHighFreqAlm) {
//						if (grpHfAlm->isDisactive()) {
//							*grpIsHighFreqAlm = false;
//							cout << endl << "High Freq Alarm DisActive ["<< id << "]" << endl << endl;
//							save2SC = true;
//							almValOut = false;
//							setChanged(0, false);
//						} else
//							save2SC = false;
//					} else {
//						save2SC = true;
//						almValOut = false;
//						setChanged(0, false);
//					}
//				}
//			}
//		} else {
//			if ((semaIdx2_0 < grpNum) &&  isChanged(semaIdx2_0)) {
//				if (grpFsuVal[semaIdx2_0] > 0) {// 产生
//					almValOut = true;
//					setChanged(semaIdx2_0, false);
//					if (!grpIsHighFreqAlm[semaIdx2_0]) {
//						if(	(grpIsHighFreqAlm[semaIdx2_0] = grpHfAlm[semaIdx2_0].addNewAndCheckActive(time(NULL))))
//							cout << endl << "High Freq Alarm Active [" << id << "]" << endl << endl;
//						cout << "alm out " << id << endl;
//						save2SC = true;
//					} else {
//						save2SC = false;
//						grpHfAlm[semaIdx2_0].setOnState();
//					}
//				} else {						// 恢复
//					if (!almReported(semaIdx2_0)) {
//						save2SC = false;
//						almValOut = false;
//						setChanged(semaIdx2_0, false);
//					} else {
//						if (grpIsHighFreqAlm[semaIdx2_0]) {// 高频告警时
//							if (grpHfAlm[semaIdx2_0].isDisactive()) {// 高频恢复
//								grpIsHighFreqAlm[semaIdx2_0] = false;
//								cout << endl << "High Freq Alarm DisActive ["<< id << "]" << endl << endl;
//								save2SC = true;
//								almValOut = false;
//								setChanged(semaIdx2_0, false);
//							} else {
//								save2SC = false;
//							}
//						} else {
//							save2SC = true;
//							almValOut = false;
//							setChanged(semaIdx2_0, false);
//						}
//					}
//				}
//			}
//		}
//#endif
//		if (isChanged(idx2)) {
//			if (idx2 < grpNum) {
//				if (grpFsuVal[idx2] > 0) {// 产生
//					almValOut = true;
//					setChanged(idx2, false);
//					if (!grpIsHighFreqAlm[idx2]) {
//						if(	(grpIsHighFreqAlm[idx2] = grpHfAlm[semaIdx2_0].addNewAndCheckActive(time(NULL))))
//							cout << endl << "High Freq Alarm Active [" << id << "]" << endl << endl;
//						cout << "alm out " << id << endl;
//						save2SC = true;
//					} else {
//						save2SC = false;
//						grpHfAlm[idx2].setOnState();
//					}
//				} else {						// 恢复
//					if (!almReported(idx2)) {
//						save2SC = false;
//						almValOut = false;
//						setChanged(idx2, false);
//					} else {
//						if (grpIsHighFreqAlm[idx2]) {// 高频告警时
//							if (grpHfAlm[idx2].isDisactive()) {// 高频恢复
//								grpIsHighFreqAlm[idx2] = false;
//								cout << endl << "High Freq Alarm DisActive ["<< id << "]" << endl << endl;
//								save2SC = true;
//								almValOut = false;
//								setChanged(idx2, false);
//							} else {
//								save2SC = false;
//							}
//						} else {
//							save2SC = true;
//							almValOut = false;
//							setChanged(idx2, false);
//						}
//					}
//				}
//			}
//		}
//	}
//	uuu
//	return save2SC;
//}
int DI::getSN(u32 semaIdx2_0) {
	mtx_sn.lock();
	int rtn = grpSN[semaIdx2_0];
	mtx_sn.unlock();
	return rtn;
}
void DI::setSN(u32 semaIdx2_0, u32 sn) {
	mtx_sn.lock();
	grpSN[semaIdx2_0] = sn;
	mtx_sn.unlock();
}
bool DI::almReported(u32 semaIdx2_0) {
	int sn = getSN(semaIdx2_0);
	return (sn != -1);
}
void DI::setVal(u32 semaIdx2_0, u32 val) {
#if DEBUG_DAT_DETAIL
	string sid = semaIdCalcAdd(id, semaIdx2_0);
	cout << "\t\t\tDI::SetVal([" << sid << "]," << val << ")" << endl;
#endif
//	if (refAIId.empty() || strThreshold.empty()) { // 非FSU 判断，存设备返回的值
	if (!isMemAlloc())
		return;
//	ssl
	if (!bGrp) {
		mtx.lock();
		lll
		*grpFsuVal = val;
		uuu
		mtx.unlock();
		setAvail(0);
	} else {
		if ((semaIdx2_0 >= 0) && (semaIdx2_0 < grpNum)) {
//			string s = gDat.semaIdPreString + semaIdCalcAdd(id, semaIdx2_0);
			mtx.lock();
			lll
//			cout<<"DI::setVal("<< s
//					<< "," << val << ")"<< endl;
			grpFsuVal[semaIdx2_0] = val;
			uuu
			mtx.unlock();
			setAvail(semaIdx2_0);
		}
	}
//	}
//	ssu
}
void DI::devSetVal(u32 semaIdx2_0, u32 val) {
//	return;
#if DEBUG_DAT_DETAIL
	string sid = semaIdCalcAdd(id, semaIdx2_0);
	cout << "\t\t"<< __FUNCTION__ << "[" << sid << "] = " << val << endl;
#endif
//	if (id == "07005001")
//		cout << "now!" << endl;
	if (!isMemAlloc()) return;
	if (refAIId.empty() || !isFsuCtrl()) { // 非FSU 判断，存设备返回的值
		u32 idx2 = (bGrp) ? semaIdx2_0 : 0;
		if (idx2 < grpNum) {
			mtx.lock();
			lll
			grpDevVal[idx2] = val;
			if (level == 0)
				grpFsuVal[idx2] = val;
			uuu
			mtx.unlock();
			setAvail(idx2);
		}
//		if (!bGrp) {
//			mtx.lock();
//			lll
//			*grpDevVal = val;
//			if (level == 0)
//				*grpFsuVal = val;
//			uuu
//			mtx.unlock();
//			setAvail(0);
//		} else {
//			if ((semaIdx2_0 >= 0) && (semaIdx2_0 < grpNum)) {
//				mtx.lock();
//				lll
//				grpDevVal[semaIdx2_0] = val;
//				if (level == 0)
//					grpFsuVal[semaIdx2_0] = val;
//				uuu
//				mtx.unlock();
//				setAvail(semaIdx2_0);
//			}
//		}
	}
}
void DI::devSetValHdl(const string & semaId, u32 val) {
//	cout << __FUNCTION__ << ":"<< semaId << ":" << val << endl;
	u32 semaIdx2_0 = getSemaphoreIdx2(semaId) - 1;
	if(isMemAlloc()) {
		if (!isAvail(semaIdx2_0))
			setAvail(semaIdx2_0);
		devSetVal(semaIdx2_0, val);
	}
}
bool DI::isFsuCtrl() {
	bool rtn;
	mtx_threshold.lock();
	rtn = (strThreshold.size() > 0);
	mtx_threshold.unlock();
	return rtn;
}
bool DI::setThreshold(string val) {
	bool rtn = false;
	RLock(mtx_threshold);
	lll
	if (!val.empty()) {
		if (isStrDatOk(val)) {
			strThreshold = val;
			threshold = str2Float(strThreshold);
			rtn = true;
		} else
			cout << "Bad format of DI's threshold set!" << endl;
	}
//	else	LOG("empty string set !");
	uuu
	return rtn;
}
string DI::getThreshold() {
	string rtn;
	mtx_threshold.lock();
	lll
	rtn = strThreshold;
	uuu
	mtx_threshold.unlock();
	return rtn;
}
void DI::fsuSetVal(/*const string & devFullId,*/ u32 semaIdx2_0, bool bRevers, bool bAlm) {
#if DEBUG_DAT_DETAIL
	string sid = semaIdCalcAdd(id, semaIdx2_0);
	cout << "\t\t"<< __FUNCTION__ << ":([" << sid << "],"
			<< (u32)bRevers <<","<< (u32)bAlm << ")" << endl;
#endif
//	if (bAlm) {
//		string semId = gDat.semaIdPreString + id;
//		pair<string, string> pr = make_pair(devFullId, semId);
//		if (DB.isInLastActiveAlmDB(pr)) {
//			setVal(semaIdx2_0, 1);
//#if DEBUG_ALM_CHK_DETAIL
//			cout << __FUNCTION__ << ":synced last active alarm, dev_id(" << devFullId << ", " << semId << ")" << endl;
//#endif
//			setChanged(semaIdx2_0, false);
//			return;
//		}
//	}
	setVal(semaIdx2_0, bAlm);
	if (bRevers) {
		setChanged(semaIdx2_0, true);
	}
}
void DI::fsuChkReverse(/*const string & devFullId,*/ u32 semaIdx2_0, float ref) {
#if DEBUG_DAT_DETAIL
	string sid = semaIdCalcAdd(id, semaIdx2_0);
	cout << __FUNCTION__ << ":[" << sid << "]," << endl;
#endif
//	if (id == "07005001")
//		cout << "now!" << endl;

	u32 * _pCnt = grpCnt + semaIdx2_0;
	float * _pAiValWhenAlmOnOff = grpAiVal + semaIdx2_0;
#if DEBUG
	string fid = semaIdCalcAdd(id, semaIdx2_0);
#endif
#if DEBUG_DAT_DETAIL
	cout << "\t # " <<  __FUNCTION__ << "()" << fid <<",ref = " << ref << endl;
#endif
	u8 val = 0;
	bool avail = getVal(semaIdx2_0, val);
	if (!avail) return;

	mtx.lock();
	lll
	bool bAlm = (val == 1);
	bool bRevers = false;

//	if ((id == "06019001") && (semaIdx2_0 == 1))	// 同步了告警就不会再产生一次了
//		cout << "now!\n";

	if (iLogic == 1) {
		if (bAlm) {
			if (ref < (threshold - backlash)) { // 恢复
				*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
				cout << __FUNCTION__ << " alarm disActive check [(>)" << fid << "] delay=" << almOffDelay << ", \tcount=" << *_pCnt << endl;
				cout << "rev < (threshold - backlash):" << ref << " < (" << threshold << " - " << backlash << endl;
#endif
				if (*_pCnt > almOffDelay) { // 翻转, 恢复延时按标准固定20s
					bRevers = true;
					*_pCnt = 0;
					*_pAiValWhenAlmOnOff = ref;
				}
			} else {
				*_pCnt = 0; // clear counters
			}
		} else {
			if (ref >= threshold) { // 产生
				if (backlash > 0) { // 有回差直接告警不延迟
					bRevers = true;
				} else {
					*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
					cout << __FUNCTION__ << " alarm active check [(>)" << fid << "] delay=" << delay << ", \tcount=" << *_pCnt << endl;
					cout << "rev >= threshold : " << ref << " >= " << threshold << endl;
#endif
					if (*_pCnt > delay)  // 翻转
						bRevers = true;
				}
			} else {
				*_pCnt = 0; // clear counters
			}
		}
	} else if (iLogic == -1){
		if (bAlm) {
			if (ref > (threshold + backlash)) { // 恢复
				*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
				cout << __FUNCTION__ << " alarm disActive check [(<)[" << fid << "] delay=" << almOffDelay << ", \tcount=" << *_pCnt << endl;
				cout << "rev > (threshold + backlash):" << ref << " < (" << threshold << " + " << backlash << endl;
#endif
				if (*_pCnt > almOffDelay) { // 翻转

					bRevers = true;
				}
			} else {
				*_pCnt = 0; // clear counters
			}
		} else {
			if (ref <= threshold) { // 产生
				if (backlash > 0) { // 有回差直接告警不延迟
					bRevers = true;
				} else {
					*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
					cout << __FUNCTION__ << " alarm active check [(<)" << fid << "] delay=" << delay << ", \tcount=" << *_pCnt << endl;
					cout << "ref <= threshold :" << ref << " <= " << threshold << endl;
#endif
					if (*_pCnt > delay) { // 翻转
						bRevers = true;
					}
				}
			} else {
				*_pCnt = 0; // clear counters
			}
		}
	} else {
		// 0
	}
	if (bRevers) {
		bAlm = !bAlm;
		*_pCnt = 0;
		*_pAiValWhenAlmOnOff = ref;
#if DEBUG_ALM_CHK_END
		cout << endl << "fsuChkReverse alarm reverse:[" << fid << "]" << endl << endl;
#endif
	}
	uuu
	mtx.unlock();
	fsuSetVal(/*devFullId, */semaIdx2_0, bRevers, bAlm);
}
void DI::fsuCalcReverse(/*const string & devFullId,*/ u32 semaIdx2_0, bool rslt, float ref) {
	if (level <= 0) return; // 不上报的
	if (!isMemAlloc()) return;
	mtx.lock();
	lll
#if 0//DEBUG_DAT_DETAIL
	cout << "ref = " << ref << endl;
#endif
	u32 * _pCnt = grpCnt + semaIdx2_0;
	float * _pAiValWhenAlmOnOff = grpAiVal + semaIdx2_0;
	u8 val = 0;
	uuu
	mtx.unlock();
	bool avail = getVal(semaIdx2_0, val);
	bool bAlm;
	if (avail)
		bAlm = (val == 1);
	else {
		bAlm = false;// 未使用过，默认false
		setVal(semaIdx2_0, 0);
		return;
	}
	mtx.lock();
	lll
	bool bRevers = false;
	if (bAlm) {
		if (!rslt) { // 恢复
			*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
			cout << __FUNCTION__ << " alarm disActive check [" << id << "] delay=" << almOffDelay << ", \tcount=" << *_pCnt << endl;
#endif
			if (*_pCnt > almOffDelay) { // 翻转, 恢复延时按标准固定20s
				bRevers = true;
#if DEBUG_ALM_CHK_END
				cout << endl << "[ - ]" << __FUNCTION__ << " alarm disactive: [" << id << "]" << endl << endl;
#endif
			}
		} else {
			*_pCnt = 0;
		}
	} else {
		if (rslt) { // 产生
			*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
			cout << __FUNCTION__ << " alarm active check [" << id << "] delay=" << delay << ", \tcount=" << *_pCnt << endl;
#endif
			if (*_pCnt > delay) { // 翻转
				bRevers = true;
#if DEBUG_ALM_CHK_END
				cout << endl << "[ + ]" << __FUNCTION__ << " alarm active: [" << id << "]" << endl << endl;
#endif
			}
		} else {
			*_pCnt = 0;
		}
	}
	if (bRevers) {
		bAlm = !bAlm;
		*_pCnt = 0;
		*_pAiValWhenAlmOnOff = ref;
	}
	uuu
	mtx.unlock();
	fsuSetVal(/*devFullId,*/ semaIdx2_0, bRevers, bAlm);
}
void DI::devChkReverse(u32 semaIdx2_0) {
	RLock(mtx);
	lll
	u8 * _pOldVal = grpFsuVal + semaIdx2_0;

	u32 * _pCnt = grpCnt + semaIdx2_0;
	float * aiValWhenAlmOnOff = grpAiVal + semaIdx2_0;
	string refId = semaIdCalcAdd(refAIId, semaIdx2_0);
	u32 newVal = *(grpDevVal + semaIdx2_0);
#if DEBUG
	string sid = semaIdCalcAdd(id, semaIdx2_0);
#endif
#if DEBUG_ALM_CHK
	if (((u32)*_pOldVal) != newVal)
		cout << __FUNCTION__ << " alarm active check - " << "id=" <<  sid << ",old=" << (int)*_pOldVal
		 	 << ", new=" << newVal << endl;
#endif
	bool bAlm = (*_pOldVal == 1);
	if (bAlm) {
		if (newVal == 0){ // 恢复
			*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
			cout << __FUNCTION__ << " alarm disActive check - id[" << sid << "] delay=" << almOffDelay << ",count=" << *_pCnt << endl;
#endif
			if (*_pCnt > almOffDelay) { // 翻转 std = 20
				*_pOldVal = 0;
				*_pCnt = 0;
				float ref;
				if(upDev->getAIValById(refId, ref))
					*aiValWhenAlmOnOff = ref;
				else
					*aiValWhenAlmOnOff = 0;		// may be no data !!!
				setChanged(semaIdx2_0, true);
#if DEBUG_ALM_CHK_END
				cout << endl << "[ - ]" << __FUNCTION__ << " alarm disactive:id[" << sid << "]" << endl<< endl;
#endif
			}
		} else
			*_pCnt = 0; // clear counters
	} else {
		if (newVal == 1) { // 产生
			*_pCnt += 1;
#if DEBUG_ALM_CHK_PROC
			cout << __FUNCTION__ << " alarm active check - id[" << sid <<"] delay=" << delay << ", count=" << *_pCnt << endl;
#endif
			if (*_pCnt > delay) { // 翻转
				*_pOldVal = 1;
				*_pCnt = 0;
				float ref;
				if(upDev->getAIValById(refId, ref))
					*aiValWhenAlmOnOff = ref;
				else
					*aiValWhenAlmOnOff = 0;		// may be no data !!!
				setChanged(semaIdx2_0, true);
#if DEBUG_ALM_CHK_END
				cout << endl << "[ + ]" << __FUNCTION__ << " alarm active:id[" << sid << "]" << endl << endl;
#endif
			}
		} else {
//			if (sid == "06001001")
//				cout << "now\n";
			*_pCnt = 0; // clear counters
		}
	}
	uuu
}
void DI::check(const string & devFullId) { // every second !
//	cout << "check in\n";
	if (level <= 0) return; // 不上报的
	if (!isMemAlloc()) return;
//	cout << __FUNCTION__ << ":" << id << endl;
//	if (id == "06018001")
//		cout << "now!" << endl;
//	if (id == "07005001")
//		cout << "now!" << endl;
	if (isFsuCtrl()) {
		if (!refAIId.empty()) { // FSU 判断，不参考设备获取的值
			float ref;
			if (bGrp) {
				for (u32 i = 0; i < grpNum; ++i) {
					string refId = semaIdCalcAdd(refAIId, i);
					if (upDev->getAIValById(refId, ref)) {
						setAvail(i);
//						cout << __FUNCTION__ << ":" << id << endl;
						fsuChkReverse(/*devFullId,*/ i, ref);
					}
					else
						cout << __FUNCTION__ << "：can't get ref AI(" << refId << ") val\n";
				}
			} else {
				if (upDev->getAIValById(refAIId, ref))
					setAvail(0);
					fsuChkReverse(/*devFullId, */0, ref);
//				else
//					cout << __FUNCTION__<< "：can't get ref AI(" << refAIId << ") val\n";
			}
		} else { // ref is null, check at other place

		}
	} else { // 设备上报的告警，直接做延时处理
		if (bGrp) {
			for (u32 i = 0; i < grpNum; ++i)
				if (isAvail(i))
					devChkReverse(i);
		} else {
			if (isAvail(0)) {
				devChkReverse(0);
			}
		}
	}
//	cout << "check out\n";
}
bool DI::isMemAlloc() {
	mtx_mem.lock();
	bool rtn = bMalloc;
	mtx_mem.unlock();
	return rtn;
}
void DI::setMemAlloc() {
	mtx_mem.lock();
	bMalloc = true;
	mtx_mem.unlock();
	// sync old alarm.
	for (u32 i = 0; i < grpNum; ++i) {
//		cout << "sync:" << upDev->fullId << "."
//				<< gDat.semaIdPreString + semaIdCalcAdd(id, i) << endl;
		string sn;
		if (DB.isInLastActiveAlmDB(make_pair(upDev->fullId,
				gDat.semaIdPreString + semaIdCalcAdd(id, i)), sn)) {
#if 1//DEBUG_ALM_CHK_DETAIL
			cout << endl <<__FUNCTION__ << ":sync last active alarm, "
					<< upDev->fullId << "."
					<< gDat.semaIdPreString + semaIdCalcAdd(id, i) << endl << endl;
#endif
			grpSN[i] = atoi(sn.c_str());
			setVal(i, 1);
		}
	}
}
bool DI::isChanged(u32 semaIdx2_0) {
	mtx_chg.lock();
	bool rtn = *(grpChanged + semaIdx2_0);
	mtx_chg.unlock();
	return rtn;
}
void DI::setChanged(u32 semaIdx2_0, bool val) {
	RLock(mtx_chg);
	*(grpChanged + semaIdx2_0) = val;
}
//bool DI::isLastActWasFiltered(u32 semaIdx2_0) {
//	mtx_filter.lock();
//	bool rtn = *(grpLastActWasFiltered + semaIdx2_0);
//	mtx_filter.unlock();
//	return rtn;
//}
//void DI::setLastActWasFiltered(u32 semaIdx2_0, bool newVal) {
//	RLock(mtx_filter);
//	*(grpLastActWasFiltered + semaIdx2_0) = newVal;
//}
bool DI::isFilter() {
	mtx_filter.lock();
	bool rtn =filter;
	mtx_filter.unlock();
	return rtn;
}
void DI::setFilter(bool newVal) {
//	cout << __FUNCTION__ << ":" << id << "," << (u32)newVal << endl;
	mtx_filter.lock();
	filter = newVal;
	mtx_filter.unlock();
}
void DI::setAvail(u32 semaIdx2_0) {
	RLock(mtx_avail);
	*(grpAvail + semaIdx2_0) = true;
}
bool DI::isAvail(u32 semaIdx2_0) {
	mtx_avail.lock();
	bool rtn = *(grpAvail + semaIdx2_0);
	mtx_avail.unlock();
	return rtn;
}
//-----------------------------------------------------
AI::AI() {
	thresholdType = 0;
	bMalloc = false;
	bGrp = false;
	grpNum = 0;
	grpDat = 0;
	threshold = 0;
//	lastGrpDat = NULL;
	grpDat = NULL;
	grpAvail = NULL;

//	pthread_mutex_init(&mutex_lock, NULL);
}
AI::~AI() {
	if (!isMemAlloc()) return;
	if (bGrp) {
		delete[] grpDat;
		delete[] grpAvail;
		grpDat = NULL;
		grpAvail = NULL;
	} else {
		delete grpDat;
		delete grpAvail;
		grpDat = NULL;
		grpAvail = NULL;
	}
}
void AI::init(const stInitAI & sema) {
	name = sema.name;
	id = sema.id;
	unit = sema.unit;
#if DEBUG_INIT_DAT
	cout << "AI(" << sema.name << " - "<< id << ") 's unit is [ " << unit << "]" << endl;
#endif
	if (sema.threshold_abs.size() > 0) {
		thresholdType = 1;
		strThreshold = sema.threshold_abs;

	} else if (sema.threshold_rel.size() > 0) {
		thresholdType = 2;
		strThreshold = sema.threshold_rel;
	} else
		thresholdType = 0;
	bGrp = sema.bGrp;
	if (bGrp) {
		grpNum = 0;
		if (sema.bFixNum) {
			grpNum = sema.grpNum;
#if DEBUG_INIT_DAT
			cout << "in AI_Init(), id = " << id << ", grpNum = " << grpNum
			<< endl;
#endif
//			lastGrpDat = new string[grpNum];
			grpDat = new string[grpNum];
			grpAvail = new bool[grpNum];
			for (u32 i = 0; i < grpNum; ++i)
				grpAvail[i] = false;
			bMalloc = true;
		} else {
//			grpRefId = sema.gId;
			bMalloc = false;
		}
	} else {
//		lastGrpDat = new string();
		grpNum = 1;
		grpDat = new string();
		grpAvail = new bool;
		*grpAvail = false;
		bMalloc = true;
	}

//	pthread_mutex_init(&mutex_lock, NULL);
}
void AI::updateGrpSize(u32 size) {
	if (!bGrp)
		return;
	mtx_ai.lock();
	al
	if (size > grpNum) {
		if (isMemAlloc()) {
			delete[] grpDat;
			delete[] grpAvail;
		}
		// 分配内存
		grpDat = new string[size];
//		lastGrpDat = new string[size];
		grpAvail = new bool[size];
		for (u32 i = 0; i < size; ++i)
			grpAvail[i] = false;
		setMemAlloc();
	}
	grpNum = size;
	au
	mtx_ai.unlock();
}
bool AI::getValStr(u32 semaIdx2_0, string & out) {
	if (!isMemAlloc() || !isAvail(semaIdx2_0)) return false;

	if (!bGrp) {
		mtx_ai.lock();
		al
		out = *grpDat;
		au
		mtx_ai.unlock();
		return true;
	} else {
		if (semaIdx2_0 < grpNum) {
			mtx_ai.lock();
			al
			out = grpDat[semaIdx2_0];
			au
			mtx_ai.unlock();
			return true;
		}
	}
	return false;
}
bool AI::getGrpValStr(vector<string> & out) {
	if (isMemAlloc()) {
		mtx_ai.lock();
		al
		for (u32 i = 0; i < grpNum; ++i) {
			if (!isAvail(i))
				continue;
			out.push_back(grpDat[i]);
		}
		au
		mtx_ai.unlock();
	} else
		return false;
	return true;
}
string AI::getUnit() {
	return unit;
}
void AI::setValStr(u32 semaIdx2_0, string strVal) {
	if (!isMemAlloc()) return;
//	if (id == "06125001")
//		cout << "now!" << endl;
	setAvail(semaIdx2_0);
	if (!bGrp) {
		mtx_ai.lock();
		al
		*grpDat = strVal;
		au
		mtx_ai.unlock();
	} else {
		if ((semaIdx2_0 >= 0) && (semaIdx2_0 < grpNum)) {
			mtx_ai.lock();
			al
			grpDat[semaIdx2_0] = strVal;
			au
			mtx_ai.unlock();
		}
	}
}
void AI::setAvail(u32 semaIdx2_0) {
	RLock(mtx_avail);
	*(grpAvail + semaIdx2_0) = true;
}
bool AI::isAvail(u32 semaIdx2_0) {
	mtx_avail.lock();
	bool rtn = *(grpAvail + semaIdx2_0);
	mtx_avail.unlock();
	return rtn;
}
bool AI::isMemAlloc() {
	mtx_mem.lock();
	bool rtn = bMalloc;
	mtx_mem.unlock();
	return rtn;
}
void AI::setMemAlloc() {
	mtx_mem.lock();
	bMalloc = true;
	mtx_mem.unlock();
}
bool AI::setThreshold(string absThrd, string relThrd) {
	bool rtn = false;
#if DEBUG
	cout << "input: abs = \'" << absThrd << "\', rel = \'" << relThrd << "\'" << endl;
#endif
	bool datOk1 = true;
//	bool datOk2 = false;
	string input;
	switch (thresholdType) {
	case 1:		input = absThrd;
		break;
	case 2:		input = relThrd;
		break;
	default:
		datOk1 = false;
	}
	if (datOk1) {
		mtx_ai.lock();
		lll
		if (input.size() > 0) {
			if (isStrDatOk(input)) {
				strThreshold = input;
				threshold = str2Float(strThreshold);
				rtn = true;
			}
		} else {	// empty string !
			strThreshold = input;
			rtn = true;
		}
		uuu
		mtx_ai.unlock();
	}
	return rtn;
}
void AI::getThreshold(string & absThrd, string & relThrd) {
	mtx_ai.lock();
	al
	switch (thresholdType) {
	case 1:
		absThrd = strThreshold;
		relThrd = string();
		break;
	case 2:
		absThrd = string();
		relThrd = strThreshold;
		break;
	case 0:
	default:
		absThrd = string();
		relThrd = string();
		break;
	}
	au
	mtx_ai.unlock();
}
void AI::makeSemaphore(vector<stSemaphore_l> & out) {
	if (!isMemAlloc()) return;
//	cout << "\t****" <<  __FUNCTION__ << ": 0\n";
	mtx_ai.lock();
	al
//	cout << "\t****" <<  __FUNCTION__ << ": 1\n";
	string type = AI_STR;
	string fullId = gDat.semaIdPreString + id;
	string status = string("0");
	if (bGrp) {
		for (u32 i = 0; i < grpNum; ++i) {
			if (isAvail(i))
				out.push_back(
						stSemaphore_l(type, fullId, *(grpDat + i), status));
		}
	} else {
		if (isAvail(0))
			out.push_back(stSemaphore_l(type, fullId, *grpDat, status));
	}
//	cout << "\t****" <<  __FUNCTION__ << ": 2\n";
	au
	mtx_ai.unlock();
}
//---------------------------------------------------------
void DO::init(const stInitDO & sema) {
	id = sema.semaId;
	bGrp = sema.bGrp;
	if (bGrp) {
		if (sema.bFixNum) {
			grpNum = sema.grpNum;
#if DEBUG_INIT_DAT
			cout << "in DO_Init(), id = " << id << ", grpNum = " << grpNum
			<< endl;
#endif
		}
	}
}
void DO::updateGrpSize(u32 size) {
	RLock(mtx);
	if (!bGrp)
		return;
	grpNum = size;
}
//---------------------------------------------------------
AO::~AO() {
	if (!isMemAlloc()) return;
	if (bGrp) {
		delete[] grpDat;
		delete[] grpAvail;
		grpDat = NULL;
		grpAvail = NULL;
	} else {
		delete grpDat;
		delete grpAvail;
		grpDat = NULL;
		grpAvail = NULL;
	}
}
void AO::init(const stInitAO & sema) {
	id = sema.semaId;
	bGrp = sema.bGrp;
	if (bGrp) {
		if (sema.bFixNum) {
			grpNum = sema.grpNum;
#if DEBUG_INIT_DAT
			cout << "in AO_Init(), id = " << id << ", grpNum = " << grpNum
			<< endl;
#endif
			grpDat = new string[grpNum];
			grpAvail = new bool[grpNum];
			for (u32 i = 0; i < grpNum; ++i)
				grpAvail[i] = true;		// was false
			bMalloc = true;
		} else {
			bMalloc = false;
		}
	} else {
		grpDat = new string();
		grpAvail = new bool;
		*grpAvail = true;// was false;
		bMalloc = true;
	}
}
void AO::setAvail(u32 semaIdx2_0) {
	RLock(mtx_avail);
	*(grpAvail + semaIdx2_0) = true;
}
bool AO::isAvail(u32 semaIdx2_0) {
	mtx_avail.lock();
	bool rtn = *(grpAvail + semaIdx2_0);
	mtx_avail.unlock();
	return rtn;
}
bool AO::isMemAlloc() {
	mtx_mem.lock();
	bool rtn = bMalloc;
	mtx_mem.unlock();
	return rtn;
}
void AO::setMemAlloc() {
	mtx_mem.lock();
	bMalloc = true;
	mtx_mem.unlock();
}
bool AO::getValStr(u32 semaIdx2_0, string & out) {
	if (!isMemAlloc() || !isAvail(semaIdx2_0)) return false;
	bool ok = false;
	if (!bGrp) {
		mtx.lock();
		out = *grpDat;
		mtx.unlock();
		ok = true;
	} else {
		if (semaIdx2_0 < grpNum) {
			mtx.lock();
			out = grpDat[semaIdx2_0];
			mtx.unlock();
			ok = true;
		}
	}
	return ok;
}
void AO::updateGrpSize(u32 size) {
	RLock(mtx);
	if (!bGrp)
		return;
	if (size > grpNum) {
		if (isMemAlloc()) {
			delete[] grpDat;
			delete[] grpAvail;
		}
		// 分配内存
		grpDat = new string[size];
//		lastGrpDat = new string[size];
		grpAvail = new bool[size];
		for (u32 i = 0; i < size; ++i)
			grpAvail[i] = false;
		setMemAlloc();
	}
	grpNum = size;
}
void AO::setValStr(u32 semaIdx2_0, string strVal) {
	if (!isMemAlloc()) return;
	if (!bGrp) {
		mtx.lock();
		*grpDat = strVal;
		setAvail(0);
		mtx.unlock();
	} else {
		if ((semaIdx2_0 >= 0) && (semaIdx2_0 < grpNum)) {
			mtx.lock();
			grpDat[semaIdx2_0] = strVal;
			setAvail(semaIdx2_0);
			mtx.unlock();
		}
	}
}
//===============================================================================================
void Device::inInitList(const string & sId, string & initVal) {
	for (const auto & i : pInitDat->vecInitlist) {
		if (i.semaId == sId) {
			initVal = i.setVal;
			return;
		}
	}
}
void Device::addDISemaphore(stInitDI & sema) {
	const char * type = devTypeId.c_str();
	const char * s = sema.id.c_str();
	if ((s[0] == type[0]) && (s[1] == type[1])) {
		u32 semaIdx1_1 = (s[3] - '0') * 10 + (s[4] - '0');
#if DEBUG_INIT_DAT
		cout << "Dev[" << fullId << "] init DI :" << sema.id << ", grpNum =  "
				<< sema.grpNum;
#endif
		pDIs[semaIdx1_1 - 1] = new DI();
		DI * di = pDIs[semaIdx1_1 - 1];
		di->upDev = this;	// before init(sema);
		inInitList(sema.id, sema.threshold_sel);
		di->init(sema);
	}
}
void Device::addAISemaphore(stInitAI & sema) {
	const char * type = devTypeId.c_str();
	const char * s = sema.id.c_str();
	if ((s[0] == type[0]) && (s[1] == type[1])) {
		u32 semaIdx1_1 = (s[3] - '0') * 10 + (s[4] - '0');
#if DEBUG_INIT_DAT
		cout << "Dev[" << fullId << "] init AI :" << sema.id << ", grpNum = "
				<< sema.grpNum << endl;
#endif
		pAIs[semaIdx1_1 - 1] = new AI();
		if (!sema.threshold_rel.empty())
			inInitList(sema.id, sema.threshold_rel);
		else
			inInitList(sema.id, sema.threshold_abs);
		pAIs[semaIdx1_1 - 1]->init(sema);
	}
}
void Device::addDOSemaphore(stInitDO & sema) {
	const char * type = devTypeId.c_str();
	const char * semId = sema.semaId.c_str();
	if ((semId[0] == type[0]) && (semId[1] == type[1])) {
		u32 semaIdx1_1 = (semId[3] - '0') * 10 + (semId[4] - '0');
		pDOs[semaIdx1_1 - 1] = new DO();
		pDOs[semaIdx1_1 - 1]->init(sema);
#if DEBUG_INIT_DAT
		cout << "Dev[" << fullId << "] init DO(" << sema.semaId << ")" << endl;
#endif
	}

}
void Device::addAOSemaphore(stInitAO & sema) {
	const char * type = devTypeId.c_str();
	const char * semId = sema.semaId.c_str();
	if ((semId[0] == type[0]) && (semId[1] == type[1])) {
		u32 semaIdx1_1 = (semId[3] - '0') * 10 + (semId[4] - '0');
		pAOs[semaIdx1_1 - 1] = new AO();
		pAOs[semaIdx1_1 - 1]->init(sema);
#if DEBUG_INIT_DAT
		cout << "Dev[" << fullId << "] init AO(" << sema.semaId << "), grpNum = " << sema.grpNum << endl;
#endif
	}
}
void Device::upDateSemaphoes(vector<stIdVal> & vals) {
//	const char * s;
	u32 type;
	u32 semaIdx1_1;
	u32 semaIdx2_1;
//	cout << "\t****" <<  __FUNCTION__ << ": 1\n";
	for (auto & pair : vals) {
//		s = pair.id.c_str();
//		size_t sz = pair.id.size();
//		if (sz == 3) { // just nxx
//			semaIdx1_1 = (s[1] - '0') * 10 + (s[2] - '0');
//			if ((semaIdx1_1 > 0) && (semaIdx1_1 < maxNotFixGrpNum))
//				pNotFixGrpNum[semaIdx1_1 - 1] = atoi(pair.val.c_str());
//		}
//		cout << "\t****" <<  __FUNCTION__ << ": 2\n";

//		cout << __FUNCTION__ << "{" << pair.id.c_str() << "," << pair.val.c_str() << "}\n";
		u32 idSz = pair.id.size();
		if (idSz == 8) {
			type = getSemaphoreType(pair.id);
			semaIdx1_1 = getSemaphoreIdx(pair.id);
			if (semaIdx1_1 <= 0) {
				cout << "bad id of semaphores" << endl;
				continue;
			}
			semaIdx2_1 = getSemaphoreIdx2(pair.id);
			if (semaIdx2_1 <= 0) {
				cout << "bad idx of semaphores" << endl;
				continue;
			}

			switch (type) {
			case T_SEMA_DI:
				if ((semaIdx1_1 - 1) < pInitDat->maxSema[T_SEMA_DI]) {
					DI * p = pDIs[semaIdx1_1 - 1];
					if (p != NULL) {
#if DEBUG_DAT_DETAIL
						cout << "\t\t\t\t o o o o o   val=(" << pair.val << ")" << endl;
#endif
						if (!pair.val.empty()) {
							u32 val;
							if (p->level == 0) {
								val = atoi(pair.val.c_str());
							} else {
								val = (atoi(pair.val.c_str())) ? 1 : 0;
							}
#if DEBUG_DAT_DETAIL
							cout << "\t\t\t\t o o after atoi()   val=(" << val << ")" << endl;
#endif
							if (p->bGrp) {
								if (p->isMemAlloc()) {
									devSetDIVal(pair.id, val);
#if DEBUG_DAT_DETAIL
									cout << "\t\t\t++ got DI(" << pair.id << "), val = " << pair.val << endl;
#endif
								}
							} else {
								devSetDIVal(pair.id, val);
#if DEBUG_DAT_DETAIL
								cout << "\t\t\t++ got DI(" << pair.id << "), val = " << pair.val << endl;
#endif
							}

						}
					}
				}
				break;
			case T_SEMA_AI:
//				if (pair.id == "06125001")
//					cout << "now!" << endl;
				if ((semaIdx1_1 - 1) < pInitDat->maxSema[T_SEMA_AI]) {
					if (pAIs[semaIdx1_1 - 1] != NULL) {
						AI * p = pAIs[semaIdx1_1 - 1];
						if (p->bGrp) {
//							if (pair.id == "06110001")
//								cout << "now\n";
							p->setValStr(semaIdx2_1 - 1, pair.val);
#if DEBUG_DAT_DETAIL
								cout << "\t\t\t++ got AI(idx1=" << semaIdx1_1 << ")[idx2="
										<<	(semaIdx2_1 - 1) << "] ,  val = " << pair.val << endl;
#endif
						} else {
							p->setValStr(0, pair.val);
#if DEBUG_DAT_DETAIL
							cout << "\t\t\t++ got AI(" << semaIdx1_1 << "),  val = " << pair.val << endl;
#endif
						}
					}
				}
				break;
			case T_SEMA_DO: // DO
				break;
			case T_SEMA_AO: // AO = set
				if ((semaIdx1_1 - 1) < pInitDat->maxSema[T_SEMA_AO]) {
					if (pAOs[semaIdx1_1 - 1] != NULL) {
						AO * p = pAOs[semaIdx1_1 - 1];
						if (p->bGrp) {
							if (p->isMemAlloc()) {
								p->setValStr(semaIdx2_1 - 1, pair.val);
#if DEBUG_DAT_DETAIL
//								cout << "\t\t\t++ got AO(idx1=" << semaIdx1_1 << ")[idx2="
//										<<	(semaIdx2_1 - 1) << "] ,  val = " << pair.val << endl;
#endif
							}
						} else {
							p->setValStr(0, pair.val);
#if DEBUG_DAT_DETAIL
//							cout << "\t\t\t++ got AO(" << semaIdx1_1 << "),  val = " << pair.val << endl;
#endif
						}
					}
				}
				break;
			}
		} else if (idSz == 3) {	// 扩展板专用
			const char * s = fullId.c_str();
			string shortId =  string(s + 7);
			u8 addr = atoi(shortId.c_str()) % 10;
			if ((addr == 0) || (addr > 8))
				continue;
			u32 id = atoi(pair.id.c_str());
			u32 idx = id % 100;
			if (id > 100) {
				DevMgr.allExtDat[addr - 1].putAIValStr(idx - 1, pair.val);
			} else {
				DevMgr.allExtDat[addr - 1].putDIVal(idx - 1, pair.val);
			}
		}
	}
//	cout << "\t****" <<  __FUNCTION__ << ": 3\n";
}
DI * Device::getDI(string shortId) {
	u32 semaIdx1_1 = getSemaphoreIdx(shortId);
	return getDI(semaIdx1_1);
}
DI * Device::getDI(u32 semaIdx1_1) {
	if (pInitDat != NULL) {
		if (semaIdx1_1 <= pInitDat->maxSema[T_SEMA_DI]) {
			if (pDIs != NULL)
				return pDIs[semaIdx1_1 - 1];
		}
	}
	return NULL;
}
AI * Device::getAI(u32 semaIdx1_1) {
	if (pInitDat != NULL) {
		if (semaIdx1_1 <= pInitDat->maxSema[T_SEMA_AI])
			if (pAIs != NULL)
				return pAIs[semaIdx1_1 - 1];
	}
	return NULL;
}
DO * Device::getDO(u32 semaIdx1_1) {
	if (pInitDat != NULL) {
		if (semaIdx1_1 <= pInitDat->maxSema[T_SEMA_DO])
			if (pDOs != NULL)
				return pDOs[semaIdx1_1 - 1];
	}
	return NULL;
}
AO * Device::getAO(u32 semaIdx1_1) {
	if (pInitDat != NULL) {
		if (semaIdx1_1 <= pInitDat->maxSema[T_SEMA_AO])
			if (pAOs != NULL)
				return pAOs[semaIdx1_1 - 1];
	}
	return NULL;
}
void Device::checkDIs() {
	if (!isReady())
		return;
	for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI]; ++semaIdx1_0) {
		DI * di = pDIs[semaIdx1_0];
		if (di) {
//			cout << "checkDI【" << semaIdx1_0 << "】"<< endl;
			di->check(fullId); // 判断 延迟、过滤...
		}
	}
}
extern AutoIncNum AlmIdx;
void Device::clearAlarm() {
	vector<stDevAlmDB> devAlarms;
	stDevAlmDB st;
	st.devId = fullId;

	for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI]; ++semaIdx1_0) {
		DI * p = pDIs[semaIdx1_0];
		if (p != NULL) {
			if (p->level <= 0)
				continue;
			if (!p->isMemAlloc())
				continue;
			if (p->bGrp) {
				for (u32 semaIdx2_0 = 0; semaIdx2_0 < p->grpNum; ++ semaIdx2_0) {
					if (p->almReported(semaIdx2_0)) {
						stAlarmDB a;
						a.sn = p->getSN(semaIdx2_0);
						a.id = gDat.semaIdPreString
								+ semaIdCalcAdd(p->id, semaIdx2_0);
						a.deviceId = fullId;
						a.deviceCode = fullId;
						a.level = gDat.almLevelStr[p->level - 1];
						a.flag = gDat.almFlag[1];
						a.desc = "clear";
						st.v.push_back(a);
					}
				}
			} else {
				if (p->almReported(0)) {
					stAlarmDB a;
					a.sn = p->getSN(0);
					a.id = gDat.semaIdPreString + p->id;
					a.deviceId = fullId;
					a.deviceCode = fullId;
					a.level = gDat.almLevelStr[p->level - 1];
					a.flag = gDat.almFlag[1];
					a.desc = "clear";
					st.v.push_back(a);
				}
			}


		}
	}
	if (st.v.size() > 0) {
		devAlarms.push_back(st);
		DB.saveNewAlarmToUnConfirmedDB(devAlarms);
	}
}
bool Device::getAlm(vector<BInt::stAlarmDB> & vec) {
	bool rtn = false;
	bool almState;
	for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI]; ++semaIdx1_0) {
		DI * p = pDIs[semaIdx1_0];
		if (p != NULL) {
			if (p->level <= 0)
				continue;
			if (!p->isMemAlloc()) continue;
			if (p->bGrp) {
				for (u32 semaIdx2_0 = 0; semaIdx2_0 < p->grpNum; ++semaIdx2_0) {
					if (!p->isAvail(semaIdx2_0))
						continue;
//					if (p->getAlarmChange(semaIdx2_0, almState)) {
					if (p->getAlarmChange2(semaIdx2_0, almState)) {
						stAlarmDB a;
						bool ok = true;
						if(almState) { // active
							u32 sn;
							rtn = AlmIdx.add(1, sn);	// XXX new grpSN
							p->setSN(semaIdx2_0, sn);
							if (rtn == false) {
								log("AlmIdx counter file handle error !\n");
								cout << "\n\t\t * * * AlmIdx counter file handle error ! * * * \n" << endl;
								ok = false;
							}
						}
						if (ok) {
							a.sn = p->getSN(semaIdx2_0);
							if (!almState)
								p->setSN(semaIdx2_0, -1);
							string idIdx = semaIdCalcAdd(p->id, semaIdx2_0);
							a.id = gDat.semaIdPreString	+ idIdx;
							string newDevId = DevMgr.changeDeviceFullId_r(fullId, idIdx);
							a.deviceId = newDevId;
							a.deviceCode = newDevId;
							a.level = gDat.almLevelStr[p->level - 1];
							a.flag = almState ? gDat.almFlag[0] : gDat.almFlag[1];
							string desc = almState ? p->almOnDesc : p->almOffDesc;
							string allDesc;
//							if (!p->refAIId.empty()) {// 通信故障告警不应该有模拟量相关数据
							if (p->strThreshold.size() > 0) {
								allDesc = desc + "("
										+ floatToString(*(p->grpAiVal + semaIdx2_0), 3, 1)
										+ p->unit + ")";
							} else
								allDesc = desc;
							if ((p->grpIsHighFreqAlm[semaIdx2_0]))
								allDesc += string("- 高频次告警");
							a.desc = allDesc;
							vec.push_back(a);
						} else {
							p->setChanged(semaIdx2_0, true);	// try again !!
#if DEBUG
							cout << "try again !\n";
#endif
						}
						rtn = ok;
					}
				}
			} else {
				if (!p->isAvail(0)) continue;
//				if (p->getAlarmChange(0, almState)) {
				if (p->getAlarmChange2(0, almState)) {
					step("after getAlarm");
					stAlarmDB a;
					bool ok = true;
					if(almState) {// active
						u32 sn;
						rtn = AlmIdx.add(1, sn);	// XXX new SN
						p->setSN(0, sn);
						if (rtn == false){
							cout << "\n\t\t * * * AlmIdx counter file handle error ! * * * \n" << endl;
							ok = false;
						}
					}
					if (ok) {
						a.sn = p->getSN(0);
						if (!almState)
							p->setSN(0, -1);
						a.id = gDat.semaIdPreString + p->id;
						string newDevId = DevMgr.changeDeviceFullId_r(fullId, p->id);
						a.deviceId = newDevId;
						a.deviceCode = newDevId;
						a.level = gDat.almLevelStr[pDIs[semaIdx1_0]->level - 1];
						a.flag = almState ? gDat.almFlag[0] : gDat.almFlag[1];
						string desc = almState ? p->almOnDesc : p->almOffDesc;
						string allDesc;
//						if (!p->refAIId.empty()) {// 通信故障告警不应该有模拟量相关数据
						if (p->strThreshold.size() > 0) {
							allDesc = desc + "(" + floatToString(*(p->grpAiVal), 3, 1)
									+ p->unit + ")";
						} else
							allDesc = desc;
						if (*(p->grpIsHighFreqAlm))
							allDesc += string(" - 高频次告警");
						a.desc = allDesc;
						vec.push_back(a);
						step("push alarm");
					} else {
						p->setChanged(0, true);	// try again !!
#if DEBUG
						cout << "try again !\n";
#endif
					}
					rtn = ok;
				}
			}
		}
	}
	return rtn;
}
bool Device::getDIStatById(u32 semaIdx1_1, u32 semaIdx2_0, u8 & out) {
	if (semaIdx1_1 <= pInitDat->maxSema[0]) { // <= ?
		DI * p = pDIs[semaIdx1_1 - 1];
		if (p)
			return p->getVal(semaIdx2_0, out);
#if DEBUG
		else {
#if DEBUG_SEMA_CHK
			cout << __FUNCTION__ << ":can't find DI(" << id << "," << semaIdx1_1 - 1 << "," << semaIdx2_0 << ")\n";
#endif
			return false;
		}
#endif
	} else
		return false;
}
bool Device::getDIsStatById(u32 semaIdx1_1, vector<u8> & out) {
	if (semaIdx1_1 <= pInitDat->maxSema[0]) { // <= ?
		DI * p = pDIs[semaIdx1_1 - 1];
		if (p)
			return p->getGrpVal(out);
#if DEBUG
		else {
#if DEBUG_SEMA_CHK
			cout << __FUNCTION__ << ": can't find DI(" << id << "," << semaIdx1_1 - 1 << ")\n";
#endif
			return false;
		}
#endif
	} else
		return false;
}
bool Device::getAIValById(u32 semaIdx1_0, u32 semaIdx2_0, float & out) {
	if ((semaIdx1_0 >= 0) && (semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI])
			&& (semaIdx2_0 >= 0)) {
		string s;
		AI * ai = pAIs[semaIdx1_0];
		if (ai) {
			if (ai->getValStr(semaIdx2_0, s)) {
				out = atof(s.c_str());
				return true;
			}
		} else {
#if DEBUG_SEMA_CHK
			cout << __FUNCTION__<< ": can't find the AI(" << id << "," << semaIdx1_0 << "," << semaIdx2_0 << ")\n";
#endif
		}
	}
	return false;
}
bool Device::getAIValById(const string &sid, float & out) {
	u32 devTypeIdx = getSemaphoreType(sid);
	if (devTypeIdx != T_SEMA_AI)
		return false;
	u32 semaIdx1_1 = getSemaphoreIdx(sid);
	u32 semaIdx2_1 = getSemaphoreIdx2(sid);
	return getAIValById(semaIdx1_1 - 1, semaIdx2_1 - 1, out);
}
bool Device::getAOValById(u32 semaIdx1_0, u32 semaIdx2_0, float & out) {
	if ((semaIdx1_0 >= 0) && (semaIdx1_0 < pInitDat->maxSema[T_SEMA_AO])
			&& (semaIdx2_0 >= 0)) {
		string s;
		AO * ao = pAOs[semaIdx1_0];
		if (ao) {
			if (ao->getValStr(semaIdx2_0, s)) {
				out = atof(s.c_str());
				return true;
			}
		} else {
#if DEBUG_SEMA_CHK
			cout << __FUNCTION__<< ": can't find the AO(" << id << "," << semaIdx1_0 << "," << semaIdx2_0 << ")\n";
#endif
		}
	}
	return false;
}
bool Device::getAOValById(const string &sid, float & out) {
	u32 devTypeIdx = getSemaphoreType(sid);
	if (devTypeIdx != T_SEMA_AO)
		return false;
	u32 semaIdx1_1 = getSemaphoreIdx(sid);
	u32 semaIdx2_1 = getSemaphoreIdx2(sid);
	return getAOValById(semaIdx1_1 - 1, semaIdx2_1 - 1, out);
}
void Device::getAIUnitById(u32 semaIdx1_0, string &u) {
	if ((semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI])
		&& pAIs[semaIdx1_0] )
		u = pAIs[semaIdx1_0]->getUnit();
	else
		u = string();
}
void Device::setDIStatById(const string &sid, u32 stat) {
//	if ("07005001" == sid)
//		cout << "now!" << endl;
	u32 devTypeIdx = getSemaphoreType(sid);
	if (devTypeIdx != T_SEMA_DI)
		return;
	u32 semaIdx1_0 = getSemaphoreIdx(sid) - 1;
	u32 semaIdx2_0 = getSemaphoreIdx2(sid) - 1;
	if ((semaIdx1_0 >= 0) && (semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI])
			&& (semaIdx2_0 >= 0)) {
		if (pDIs[semaIdx1_0])
			pDIs[semaIdx1_0]->devSetVal(semaIdx2_0, stat);
	}
}
void Device::setDIStatById(u32 semaIdx1_1, u32 semaIdx2_0, u32 stat) {
	if (semaIdx1_1 <= pInitDat->maxSema[0]) { // <= ?
		DI * p = pDIs[semaIdx1_1 - 1];
		if (p)
			p->devSetVal(semaIdx2_0, stat);
	}
}
void Device::setAIValById(const string &sid, float val) {
	u32 devTypeIdx = getSemaphoreType(sid);
	if (devTypeIdx != T_SEMA_AI)
		return;
	u32 semaIdx1_0 = getSemaphoreIdx(sid) - 1;
	u32 semaIdx2_0 = getSemaphoreIdx2(sid) - 1;
	if ((semaIdx1_0 >= 0) && (semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI])
			&& (semaIdx2_0 >= 0)) {
		if (pAIs[semaIdx1_0])
			pAIs[semaIdx1_0]->setValStr(semaIdx2_0, floatToString(val, 0, 3));
	}
}
void Device::setAIValStrById(const string & sid, string v) {
	u32 devTypeIdx = getSemaphoreType(sid);
	if (devTypeIdx != T_SEMA_AI)
		return;
	u32 semaIdx1_0 = getSemaphoreIdx(sid) - 1;
	u32 semaIdx2_0 = getSemaphoreIdx2(sid) - 1;
	if ((semaIdx1_0 >= 0) && (semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI])
			&& (semaIdx2_0 >= 0)) {
		if (pAIs[semaIdx1_0])
			pAIs[semaIdx1_0]->setValStr(semaIdx2_0, v);
	}
}
extern int fsuMode;
void DevInit::initGrpDat(const string & devName) {
	DevInit::vecInitGRP.clear();
	string p = gDat.exePath + string("cfg/grp/") + devName + string(".csv");
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return; // 可能没有组数据文件

	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 3))
			return; // 检查过了，不应该到这里
		vecInitGRP.push_back(stInitGRP(fmtSemaId1(col[0]),
				fmtSemaId1(col[1])));
	}
}
void DevInit::initADIO(const string & devName) {
	vector<vector<string> > dat;
	dat.clear();
	string p = gDat.exePath + string("cfg/base/") + devName + string(".csv");
	if (!readCsv(dat, p))
		return;		// 检查过了，不应该到这里
	vecInitDI.clear();
	vecInitAI.clear();
	vecInitDO.clear();
	vecInitAO.clear();
	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 28))
			return;	// 同上
		string sid = fmtSemaId1(col[ADIO_Col::INIT_SEMA_ID]);
//		cout << "id:" << sid;
		stInitGRP * ig = NULL;
//		cout << "group init:" << sid << endl;
		for (auto & g : vecInitGRP) {
//			cout << g.semaId << endl;
			if (sid == g.semaId) {
				ig = &g;
				break;
			}
		}
		switch(getSemaphoreType(sid)) {
		case T_SEMA_DI:
		{
			u32 siteType = atoi(gDat.siteType.c_str()) - 1;
			u32 pos = ADIO_Col::INIT_SEMA_A_ALM_CLASS + siteType * 5;
			vecInitDI.push_back(
				stInitDI(
					sid,
					fmtSemaId1(col[ADIO_Col::INIT_SEMA_AID]),
					col[ADIO_Col::INIT_SEMA_LOGIC],
					col[ADIO_Col::INIT_SEMA_BACKLASH],
					col[ADIO_Col::INIT_SEMA_NAME],
					col[ADIO_Col::INIT_SEMA_ALMDESC1],
					col[ADIO_Col::INIT_SEMA_ALMDESC0],
					col[pos + 2],// delay
					col[pos + 0],// alm class
					col[pos + 1],// threshold
					ig,
					col[ADIO_Col::INIT_SEMA_UNIT]
				)
			);
		}
			break;
		case T_SEMA_AI:
		{
//			cout << "AI:" << sid << endl;
			u32 siteType = atoi(gDat.siteType.c_str()) - 1;
			u32 pos = ADIO_Col::INIT_SEMA_A_ALM_CLASS + siteType * 5;
			vecInitAI.push_back(
				stInitAI(
					sid,
					col[pos + 3],// alm class
					col[pos + 4],// threshold
					col[ADIO_Col::INIT_SEMA_NAME],
					col[ADIO_Col::INIT_SEMA_UNIT],
					ig
				)
			);
		}
			break;
		case T_SEMA_DO:
//			cout << "DO:" << sid << endl;
			vecInitDO.push_back(
				stInitDO(
					sid,
					col[ADIO_Col::INIT_SEMA_NAME],
					ig
				)
			);
			break;
		case T_SEMA_AO:
//			cout << "AO:" << sid << endl;
			vecInitAO.push_back(
				stInitAO(
					sid,
					col[ADIO_Col::INIT_SEMA_NAME],
					col[ADIO_Col::INIT_SEMA_UNIT],
					ig
				)
			);
			break;
		}
	}
//	cout << "size:" << vecInitDI.size() << "," << vecInitAI.size() << ","
//			<< vecInitDO.size() << "," << vecInitAO.size() << "\n";
}
void DevInit::initThreshold(const string & devId) {
	string p = gDat.exePath + string("cfg/init_list.csv");
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return;	// 检查过了，不应该到这里
	const char * s = devId.c_str();
	string shortId =  string(s + 7);

	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 4))
			return;	// 检查过了，不应该到这里
		string newDevId = DevMgr.changeDeviceShortId(col[0], col[1]);
		if (shortId == newDevId)
#if DEBUG_INIT_DAT
			cout << newDevId << "<->" << col[0] << "," << col[1] << ","
			<< col[2] << "," << col[3] << endl;
#endif
			vecInitlist.push_back(stInit4ADIO(/*devId,*/
					fmtSemaId1(col[1]),	// semaId
					col[2],col[3]));
	}
}
void DevInit::setSemaAvailable(const string &id) {
#if DEBUG_INIT_DAT
	cout << "setSemaAvailable:" << id << endl;
#endif
	u32 semaType_0 = BInt::getSemaphoreType(id);
	switch (semaType_0) {
	case T_SEMA_DI: {
		for (auto & it : vecInitDI) {
			if (it.id == id)
				it.bAvail = true;
		}
	} break;
	case T_SEMA_AI: {
		for (auto & it : vecInitAI) {
			if (it.id == id)
				it.bAvail = true;
		}
	} break;
	case T_SEMA_DO: {
		for (auto & it : vecInitDO) {
			if (it.semaId == id)
				it.bAvail = true;
		}
	} break;
	case T_SEMA_AO: {
		for (auto & it : vecInitAO) {
			if (it.semaId == id)
				it.bAvail = true;
		}
	} break;
	}
}
void DevInit::initMaxMem(/*u32 devType*/) {
	maxSema[T_SEMA_DI] = 0;
	for (const auto & sema : vecInitDI) {
		u32 semaIdx1_1 = getSemaphoreIdx(sema.id);
//		cout << "vecInitDI[" << semaIdx1_1 << endl;
		if (semaIdx1_1 > maxSema[T_SEMA_DI])
			maxSema[T_SEMA_DI] = semaIdx1_1;
	}
	maxSema[T_SEMA_AI] = 0;
	for (const auto & sema : vecInitAI) {
		u32 semaIdx1_1 = getSemaphoreIdx(sema.id);
//		cout << "vecInitAI[" << semaIdx1_1 << endl;
		if (semaIdx1_1 > maxSema[T_SEMA_AI])
			maxSema[T_SEMA_AI] = semaIdx1_1;
	}
	maxSema[T_SEMA_DO] = 0;
	for (const auto & it : vecInitDO) {
		u32 semaIdx1_1 = getSemaphoreIdx(it.semaId);
//		cout << "vecInitDO[" << semaIdx1_1 << endl;
		if (semaIdx1_1 > maxSema[T_SEMA_DO])
			maxSema[T_SEMA_DO] = semaIdx1_1;
	}
	maxSema[T_SEMA_AO] = 0;
	for (const auto & it : vecInitAO) {
		u32 semaIdx1_1 = getSemaphoreIdx(it.semaId);
//		cout << "vecInitAO[" << semaIdx1_1 << endl;
		if (semaIdx1_1 > maxSema[T_SEMA_AO])
			maxSema[T_SEMA_AO] = semaIdx1_1;
	}

}
void DevInit::init0(const string & devId, const string & devName) {
	initGrpDat(devName);
	initADIO(devName);
	initThreshold(devId);
	initMaxMem();
	for (const auto & i : gDat.commDevSIdAvailTbl) {
		string modelName = findFileNameOfPath(i.modelName);
		if (modelName == devName) {
			for (const auto & j : i.ids)
				setSemaAvailable(j);
			break;
		}
	}
}
const char * DiId_dev02[5] = {"02016001", "02004001", "02003001", "02001001", "02002001"};
const char * DiId_dev06[4] = {"06016001", "06017001", "06015001", "06014001"};
const char * DiId_dev16[4] = {"16001001", "16004001", "16003001", "16002001"};

void Device::init(const string & fullId, const string & devName) {
	pInitDat = new DevInit;
	pInitDat->init0(fullId, devName);
	pthread_mutex_init(&mutex_lock_cmdList, NULL);
	vecGrpSize.clear();
	this->fullId = fullId;
//	u32 typeVal = getDeviceTypeVal(fullId);
	devTypeId = getDeviceTypeStr(fullId);

#if DEBUG_INIT_DEVICE
	for(u32 i = 0; i < 4; ++i)
		cout << "maxSema[" << i << "] = " << pInitDat->maxSema[i] << endl;
#endif
	if (pInitDat->maxSema[T_SEMA_DI] > 0)
		pDIs = new DI*[pInitDat->maxSema[T_SEMA_DI]];
	if (pInitDat->maxSema[T_SEMA_AI] > 0)
		pAIs = new AI*[pInitDat->maxSema[T_SEMA_AI]];
	if (pInitDat->maxSema[T_SEMA_DO] > 0)
		pDOs = new DO*[pInitDat->maxSema[T_SEMA_DO]];
	if (pInitDat->maxSema[T_SEMA_AO] > 0)
		pAOs = new AO*[pInitDat->maxSema[T_SEMA_AO]];

//	pNotFixGrpNum = new u32[maxNotFixGrpNum];
//	for (u32 i = 0; i < maxNotFixGrpNum; ++i)
//		pNotFixGrpNum[i] = 0;

	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DI]; ++i)
		pDIs[i] = NULL;
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AI]; ++i)
		pAIs[i] = NULL;
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DO]; ++i)
		pDOs[i] = NULL;
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AO]; ++i)
		pAOs[i] = NULL;

	u32 typeIdx = getDeviceTypeVal(fullId);
	const char ** filterTbl = NULL;
	u32 tblSize = 0;
	switch (typeIdx) {
	case 2:
		filterTbl = DiId_dev02;
		tblSize = 5;
		break;
	case 6:
		filterTbl = DiId_dev06;
		tblSize = 4;
		break;
	case 16:
		filterTbl = DiId_dev16;
		tblSize = 4;
		break;
	}
	if (filterTbl) {
		for (u32 i = 0; i < tblSize; ++i) {
			const string id = filterTbl[i];
			pInitDat->setSemaAvailable(id);
		}
	}
	//add semaphore of this type
	datFromComm = false;
	if ((typeIdx != DEV_TYPE_19_TongXinZhongDuan)// 通信才能取回数据的
	 && (typeIdx != DEV_TYPE_07_BattMeas)
	 && (typeIdx != DEV_TYPE_18_JiFangHuanJing)) {
		datFromComm = true;
	}
	if ((typeIdx == DEV_TYPE_17_DoorGuard) && (fsuMode == 2))
		datFromComm = false;

	for (auto & sema : pInitDat->vecInitDI) {
//		cout << "id=" << sema.id << endl;
		u32 devTypeId = getSemaphoreDevType(sema.id);
		if (devTypeId != typeIdx)
			continue;
		if (!sema.threshold_sel.empty())
			addDISemaphore(sema);
		else {
			if (datFromComm && (!sema.bAvail))
				continue;
			addDISemaphore(sema);
		}
	}
	for (auto & sema : pInitDat->vecInitAI) {
		u32 devTypeId = getSemaphoreDevType(sema.id);
		if (devTypeId != typeIdx)
			continue;
		if (datFromComm && (!sema.bAvail))
			continue;
		addAISemaphore(sema);
	}
	for (auto & it : pInitDat->vecInitDO) {
		u32 devTypeId = getSemaphoreDevType(it.semaId);
		if (devTypeId != typeIdx)
			continue;
		if (datFromComm && (!it.bAvail))
			continue;
		addDOSemaphore(it);
	}
	for (auto & it : pInitDat->vecInitAO) {
		u32 devTypeId = getSemaphoreDevType(it.semaId);
		if (devTypeId != typeIdx)
			continue;
		if (datFromComm && (!it.bAvail))
			continue;
		addAOSemaphore(it);
	}
	initDIsUnit();
	initNxx();
	initGrpSize();
	setReady(true);
}
void Device::initNxx() {
	for (auto & git : pInitDat->vecInitGRP) {
#if DEBUG_INIT_DAT
		cout << "grp init: fullId" << "," << git.semaId << "," << git.grpId << endl;
#endif
		if (git.grpId.size() != 3)  // just nxx .
			continue;
		initAddNxxItm(git.grpId, git.semaId);
	}
}
void Device::initGrpSize() {
	for (auto & git : pInitDat->vecInitGRP) {
#if DEBUG_INIT_DAT
		cout << "grp init: fullId" << "," << git.semaId << "," << git.grpId << endl;
#endif
		if (git.grpId.size() != 8)  // just fullId .
			continue;
		initAddGrpSizeItm(git.grpId, git.semaId);
	}
}
void Device::clearMem() {
	step(1);
	if (pDIs != NULL) {
		for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DI]; ++i) {
			if (pDIs[i] != NULL)
				delete pDIs[i];
		}
		delete [] pDIs;
	}
	step(2);
	if (pAIs != NULL) {
		for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AI]; ++i) {
			if (pAIs[i] != NULL)
				delete pAIs[i];
		}
		delete [] pAIs;
	}
	step(3);
	if (pDOs != NULL) {
		for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DO]; ++i) {
			if (pDOs[i] != NULL)
				delete pDOs[i];
		}
		delete [] pDOs;
	}
	step(4);
	if (pAOs != NULL) {
		for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AO]; ++i) {
			if (pAOs[i] != NULL)
				delete pAOs[i];
		}
		delete [] pAOs;
	}
	step(5);
	if (pInitDat != NULL)
		delete pInitDat;
	step(6);
}
void Device::initDIsUnit() {
	for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI]; ++semaIdx1_0) {
		if (pDIs[semaIdx1_0])
			pDIs[semaIdx1_0]->initUnit();
	}
	u32 typeIdx = getDeviceTypeVal(fullId);
	const char ** filterTbl = NULL;
	u32 tblSize = 0;
	switch (typeIdx) {
	case 2:
		filterTbl = DiId_dev02;
		tblSize = 5;
		break;
	case 6:
		filterTbl = DiId_dev06;
		tblSize = 4;
		break;
	case 16:
		filterTbl = DiId_dev16;
		tblSize = 4;
		break;
	}
	if (filterTbl) {
		for (u32 i = 0; i < tblSize; ++i) {
			const string id = filterTbl[i];
			u32 semaIdx1_1 = getSemaphoreIdx(id);
			DI* p = getDI(semaIdx1_1);
			if (p)
				p->setUnitForNoRefAI("V");
		}
	}
}
Device::~Device() {
#if DEBUG
	cout << __FUNCTION__ << endl;
				cout << "delete ->" << fullId << endl;
#endif
	if (pDIs) {
		for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DI];
				++semaIdx1_0) {
			if (pDIs[semaIdx1_0] != NULL)
				delete pDIs[semaIdx1_0];
		}
		delete pDIs;
	}
	if (pAIs) {
		for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI];
				++semaIdx1_0) {
			if (pAIs[semaIdx1_0] != NULL)
				delete pAIs[semaIdx1_0];
		}
		delete pAIs;
	}
	if (pDOs) {
		for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_DO];
				++semaIdx1_0) {
			if (pDOs[semaIdx1_0] != NULL)
				delete pDOs[semaIdx1_0];
		}
		delete pDOs;
	}
	if (pAOs) {
		for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_AO];
				++semaIdx1_0) {
			if (pAOs[semaIdx1_0] != NULL)
				delete pAOs[semaIdx1_0];
		}
		delete pAOs;
	}
//	delete[] pNotFixGrpNum;
	pthread_mutex_destroy(&mutex_lock_cmdList);
}
void Device::getSemaphoes_current(vector<stSemaphore_l> & in) {
	for (auto & sema : in) {
		sema.status = string("6");
		u32 semaType_0 = BInt::getSemaphoreType(sema.id);
		u32 semaIdx1_1 = BInt::getSemaphoreIdx(sema.id);
		u32 semaIdx2_1 = BInt::getSemaphoreIdx2(sema.id);
//		cout << "getSemaphoes(" << semaType_0 << "," << semaIdx1_1 << "," << semaIdx2_1 << ")"<< endl;
//		cout << "maxSema:" << pInitDat->maxSema[0] << "," << pInitDat->maxSema[1] << ","
//				<< pInitDat->maxSema[2] << "," << pInitDat->maxSema[3] << "\n";
		if (semaIdx1_1 == 0) {
			cout << "!!! wrong id !!!" << endl;
			continue;
		} else if (semaIdx1_1 <= pInitDat->maxSema[semaType_0]) { // <= ?
			switch (semaType_0) {
			case T_SEMA_DI: {
				DI *p = pDIs[semaIdx1_1 - 1];
				if (!p) {
					cout << "!!! wrong data !!!" << endl;
					continue;
				}
				sema.type = gDat.datType_DI;
				u8 val = 0;
				bool avail = p->getVal(semaIdx2_1 - 1, val);
				if (!avail)
					sema.measuredVal = "";
				else
					sema.measuredVal = (val > 0) ? DI::sAct : DI::sDisact;
//				cout << " $$$$ id:" << sema.id << ", val=" << sema.measuredVal << endl;
				sema.status = gDat.validStr;
			}
				break;
			case T_SEMA_AI: {
				AI *p = pAIs[semaIdx1_1 - 1];
				if (!p) {
					cout << "!!! wrong data !!!" << endl;
					continue;
				}
				sema.type = gDat.datType_AI;
				string val;
				bool avail = p->getValStr(semaIdx2_1 - 1, val);
				if (!avail)
					sema.measuredVal = "";
				else
					sema.measuredVal = val;
				sema.status = gDat.validStr; // 正常数据
			}
				break;
			case T_SEMA_DO: { // DO
//				DO *p = pDOs[semaIdx1_1 - 1];
//				if (!p) {
//					cout << "!!! wrong data !!!" << endl;"
//					continue;
//				}
				sema.type = gDat.datType_DO;
//				sema.status = gDat.invalidStr; // 无效数据
				sema.status = gDat.validStr;
			}
				break;
			case T_SEMA_AO: { // AO  ?
				AO *p = pAOs[semaIdx1_1 - 1];
				if (!p) {
					cout << "!!! wrong data !!!" << endl;
					continue;
				}
				sema.type = gDat.datType_AO;
				string val;
				bool avail = p->getValStr(semaIdx2_1 - 1, val);
				if (!avail)
					sema.setupVal = "";
				else
					sema.setupVal = val;
				sema.status = gDat.validStr; // 正常数据
			}
				break;
			}
		} else {
			switch (semaType_0) {
			case T_SEMA_DI:	sema.type = gDat.datType_DI;	break;
			case T_SEMA_AI:	sema.type = gDat.datType_AI;	break;
			case T_SEMA_DO:	sema.type = gDat.datType_DO;	break;
			case T_SEMA_AO:	sema.type = gDat.datType_AO;	break;
			default:
				continue;
			}
		}
	}
}
void Device::getSemaphoes_history(vector<string> & inIds,
		vector<stSemaphore> & out, string startTime, string endTime) {
	DB.getTheDeviceHistorySemaphores(fullId, inIds, out, startTime, endTime);
}
void Device::upDateHistoryDat() {
	if (!isReady())
		return;
	vector<BInt::stSemaphore_l> vecSem;
	if (!pAIs)
		return;
	for (u32 semaIdx1_0 = 0; semaIdx1_0 < pInitDat->maxSema[T_SEMA_AI]; ++semaIdx1_0) {
		AI * p = pAIs[semaIdx1_0];
		if (p)
			p->makeSemaphore(vecSem);
	}
	if (vecSem.size() > 0) {
//		cout << "%%%%%%%%% vecSem.size=" << vecSem.size() << endl;
		DB.upDateHistoryDat(fullId, fullId, vecSem);
//		vecSem.clear();
		ClearVector(vecSem);
	}
}
void Device::setPoint(const vector<stSemaphore_l> & in) {
	u32 semaType, semaIdx1_1;
	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "setPoint [+].";
#endif
//	cmds.clear();
	ClearVector(cmds);
	for (const auto & sema : in) {
		cmdItm itm;
		itm.cmdId = sema.id;
		itm.val = sema.setupVal;
		itm.stat = STAT_SETPOINT_INVALID_DAT; // invalid data
		semaType = getSemaphoreType_fullId(sema.id);
		semaIdx1_1 = getSemaphoreIdx_fullId(sema.id);
		if ((semaIdx1_1 <= pInitDat->maxSema[semaType]) && (semaIdx1_1 > 0)) {// <= ?
			switch (semaType) {
			case T_SEMA_AO: // AO = set
				if (pAOs[semaIdx1_1 - 1]) {
					itm.stat = STAT_SETPOINT_BUSY; //busy
					itm.cntErr = 0;
					cmds.push_back(itm);
				}
				break;
			case T_SEMA_DO:
				if (pDOs[semaIdx1_1 - 1]) {
					itm.stat = STAT_SETPOINT_BUSY; //busy
					itm.cntErr = 0;
					cmds.push_back(itm);
				}
				break;
			}
		}
	}
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "setPoint [-]\n";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);
}
void Device::setSetPointResult(string id, u32 result) {
	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "setSetPointResult[+]\n";
#endif
	for (auto &cmd : cmds) {
		if (cmd.cmdId == id) {
			if ((result == STAT_SETPOINT_INVALID_DAT) ||
				(result == STAT_SETPOINT_OK)) {
				cmd.cntErr = 0;
				cmd.stat = result;
#if	DEBUG_SET_CTRL_CMD_LOCK
				cout << "setSetPointResult[-]\n";
#endif
				pthread_mutex_unlock(&mutex_lock_cmdList);
				return;
			} else if (result == STAT_SETPOINT_DEV_RTN_FAIL) {
				cmd.cntErr++;
				if (cmd.cntErr > cmdItm::RETRY)
					cmd.stat = result;
			} else if ((result == STAT_SETPOINT_NO_REPONSE)
					 || (result == STAT_SETPOINT_BAD_REPONSE)){
				cmd.cntErr++;
				if (cmd.cntErr > cmdItm::MAX_ERR)
					cmd.stat = result;
			}
			break;
		}
	}
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "setSetPointResult[-]\n";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);
}
bool Device::isTaskEnd() {
	bool rtn = true;
	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "isTaskEnd[+]\n";
#endif
	for (auto &cmd : cmds) {
		if (cmd.stat == STAT_SETPOINT_BUSY) {
			rtn = false;
			break;
		}
	}
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "isTaskEnd[-]\n";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);
	return rtn;
}
void Device::getSetPointResult(vector<cmdItm> &out) {
	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "getSetPointResult[+]\n";
#endif
	out.assign(cmds.begin(), cmds.end()); //将v2赋值给v1
//	cmds.clear();
	ClearVector(cmds);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "getSetPointResult[-]\n";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);
}
bool Device::isCmdIn(string &id, string &val) {
	bool rtn = false;
	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "isCmdIn[+].";
#endif
	for (const auto c : cmds) {
		if (c.stat == STAT_SETPOINT_BUSY) {
			id = c.cmdId;
			val = c.val;
			rtn = true;
			break;
		}
	}
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "isCmdIn[-]";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);
	return rtn;
}
void Device::getThreshold(vector<stThreshold> & inOut) {
	for (auto & sema : inOut) {
//		u32 type = getSemaphoreType_fullId(sema.id);
//		u32 semaIdx1_1 = getSemaphoreIdx_fullId(sema.id);
		u32 type = getSemaphoreType(sema.id);
		u32 semaIdx1_1 = getSemaphoreIdx(sema.id);
		bool bInvalid = true;
		if ((semaIdx1_1 <= pInitDat->maxSema[type]) && (semaIdx1_1 > 0)) { // <= ?
			switch (type) {
			case T_SEMA_DI: {
				DI * pDI = pDIs[semaIdx1_1 - 1];
				if (pDI) {
					sema.type = string("2");
					sema.Threshold = pDI->getThreshold();
					sema.type = DI_STR;
					bInvalid = false;
				}
			}
				break;
			case T_SEMA_AI: {
				AI * pAI = pAIs[semaIdx1_1 - 1];
				if (pAI) {
					sema.type = string("3");
					pAI->getThreshold(sema.AbsoluteVal, sema.RelativeVal);
					sema.type = AI_STR;
					bInvalid = false;
				}
			}
				break;
			}
		}
		sema.status = bInvalid ? string("6") : string("0");
	}
}
const string & Device::getSemaName(const string & semaId) {
	u32 devTypeIdx = getSemaphoreType_fullId(semaId);
	u32 semaIdx1_1 = getSemaphoreIdx_fullId(semaId);
	if (devTypeIdx == T_SEMA_AI) {
		if (pAIs[semaIdx1_1 - 1] != NULL) {
			return pAIs[semaIdx1_1 - 1]->name;
		}
	} else if (devTypeIdx == T_SEMA_DI) {
		if (pDIs[semaIdx1_1 - 1] != NULL) {
			return pDIs[semaIdx1_1 - 1]->name;
		}
	}
	return gDat.emptyStr;

}
string Device::getShortId() {
	const char * s = fullId.c_str();
	return string(s + 7);
}
//string getSemaphoreShortId(const string & sId) {	// sId 必须是检查过的10位数字
//	const char * s = sId.c_str();
//	cout << "sid is " << sId << endl;
//	if (s[2] == '0') {
//		cout << "sid is " << s << endl;
//		cout << "sid is " << s + 3 << endl;
//		return string(s + 3);
//	}
//	else
//		return string(s + 2);
//}
void Device::setThreshold(const vector<stThreshold> & in,
		setThresholdReponseOfDevice & result) {
	result.devId = fullId;
//	result.vecSuccessItms.clear();
//	result.failedIds.clear();
	ClearVector(result.vecSuccessItms);
	ClearVector(result.failedIds);
#if DEBUG_DAT_DETAIL
//	cout << "setThreshold() ---- devId=" << fullId << endl;
#endif
	for (const auto & sema : in) {
		u32 type = getSemaphoreType_fullId(sema.id);
		u32 semaIdx1_1 = getSemaphoreIdx_fullId(sema.id);
#if DEBUG_DAT_DETAIL
//		cout << "setThreshold() ---- set type=" << type << ",semaIdx1=" << semaIdx1_1 << endl;
#endif
		if ((semaIdx1_1 >= pInitDat->maxSema[type]) || (semaIdx1_1 == 0))
			result.failedIds.push_back(sema.id);
		else {
			bool bSucc = false;
			string thrd;
			switch (type) {
			case T_SEMA_DI:
				{
				DI * pDI = pDIs[semaIdx1_1 - 1];
				if (pDI)
					bSucc = pDI->setThreshold(sema.Threshold);
					thrd = sema.Threshold;
				}
				break;
			case T_SEMA_AI:
				{
				AI * pAI = pAIs[semaIdx1_1 - 1];
				if (pAI)
					bSucc = pAI->setThreshold(sema.AbsoluteVal,
							sema.RelativeVal);
					if (pAI->thresholdType == 1)
						thrd = sema.AbsoluteVal;
					else if (pAI->thresholdType == 2)
						thrd = sema.RelativeVal;
				}
				break;
			}
			if (bSucc) {
				result.vecSuccessItms.push_back(
						thrdItm(this->getShortId(),
						sema.id,
						thrd,
						getSemaName(sema.id)));
			} else
				result.failedIds.push_back(sema.id);
		}
	}
}
void Device::getAllIds(vector<string> & ids) {
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DI]; ++i) {
		DI * p = pDIs[i];
		if (p) {
			if (p->bGrp) {
				string id;
				for (u32 j = 0; j < p->grpNum; ++j) {
					id = semaIdCalcAdd(p->id, j);
					ids.push_back(id);
#if DEBUG_XML_ID
					cout << "DI:" << id << endl;
#endif
				}
			} else {
				ids.push_back(p->id);
#if DEBUG_XML_ID
				cout << "DI:" << p->id << endl;
#endif
			}
		}
	}
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AI]; ++i) {
		AI * p = pAIs[i];
		if (p) {
			if (p->bGrp) {
				string id;
				for (u32 j = 0; j < p->grpNum; ++j) {
					id = semaIdCalcAdd(p->id, j);
					ids.push_back(id);
#if DEBUG_XML_ID
					cout << "AI:" << id << endl;
#endif
				}
			} else {
				ids.push_back(p->id);
#if DEBUG_XML_ID
				cout << "AI:" << p->id << endl;
#endif
			}
		}
	}
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_DO]; ++i) {
		DO * p = pDOs[i];
		if (p) {
			if (p->bGrp) {
				string id;
				for (u32 j = 0; j < p->grpNum; ++j) {
					id = semaIdCalcAdd(p->id, j);
					ids.push_back(id);
#if DEBUG_XML_ID
			cout << "DO:" << id << endl;
#endif
				}
			} else {
				ids.push_back(p->id);
#if DEBUG_XML_ID
			cout << "DO:" << p->id << endl;
#endif
			}
		}
	}
	for (u32 i = 0; i < pInitDat->maxSema[T_SEMA_AO]; ++i) {
		AO * p = pAOs[i];
		if (p) {
			if (p->bGrp) {
				string id;
				for (u32 j = 0; j < p->grpNum; ++j) {
					id = semaIdCalcAdd(p->id, j);
					ids.push_back(id);
#if DEBUG_XML_ID
			cout << "AO:" << id << endl;
#endif
				}
			} else {
				ids.push_back(p->id);
#if DEBUG_XML_ID
			cout << "AO:" << p->id << endl;
#endif
			}
		}
	}
}
void Device::initAddNxxItm(string gId, string semaId) {
	const char * id = semaId.c_str();
	u32 ty = id[2] - '0';
	u32 idx1 = (id[3] - '0') * 10 + (id[4] - '0');
	vecNxxGrpSize.push_back(stGrpSizeItm(gId, ty, idx1 - 1));
}
void Device::initAddGrpSizeItm(string gId, string semaId) {
	const char * id = semaId.c_str();
	u32 ty = id[2] - '0';
	u32 idx1 = (id[3] - '0') * 10 + (id[4] - '0');
	vecGrpSize.push_back(stGrpSizeItm(gId, ty, idx1 - 1));
}
void Device::updateNxxGrpSize(string id, u32 xx) {
	for (auto & it : vecNxxGrpSize) {
		if (it.gId == id) {
			if (xx != it.grpSize) {
#if DEBUG_PACK_PARSE_DETAIL
				if (gDat.args.en_parse) {
					cout << "type=" << it.ty << ",semaId(" << it.idx1 << ")'s gId(";
					cout << id << ")'s group size changed from "
						<< it.grpSize << " to " << xx << endl;
				}
#endif
				it.grpSize = xx;
				if (it.ty == 0) {
					if (pDIs[it.idx1])
						pDIs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 1) {
					if (pAIs[it.idx1])
						pAIs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 2) {
					if (pDOs[it.idx1])
						pDOs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 3) {
					if (pAOs[it.idx1])
						pAOs[it.idx1]->updateGrpSize(xx);
				}
			}
		}
	}
}
void Device::updateGrpSize(string id, u32 xx) {
//	cout << endl << "\t * updateGrpSize() to " << xx << endl << endl;
	for (auto & it : vecGrpSize) {
		if (it.gId == id) {
			if (xx != it.grpSize) {
#if DEBUG_PACK_PARSE_DETAIL
				if (gDat.args.en_parse) {
					cout << "type=" << it.ty << ",semaId(" << it.idx1 << ")'s gId(";
					cout << id << ")'s group size changed from "
						<< it.grpSize << " to " << xx << endl;
				}
#endif
				it.grpSize = xx;
				if (it.ty == 0) {
					if (pDIs[it.idx1])
						pDIs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 1) {
					if (pAIs[it.idx1])
						pAIs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 2) {
					if (pDOs[it.idx1])
						pDOs[it.idx1]->updateGrpSize(xx);
				} else if (it.ty == 3) {
					if (pAOs[it.idx1])
						pAOs[it.idx1]->updateGrpSize(xx);
				}
			}
		}
	}
}
bool Device::isGrpSizeId(string id) {
//	cout << endl << "\t * isGrpSizeId()" << endl << endl;

	for (auto & it : vecGrpSize) {
		if (it.gId == id)
			return true;
	}
	return false;
}
bool Device::isGrpSema(string id) {
	u32 idx1_1 = getSemaphoreIdx_fullId(id);
	u32 type = getSemaphoreType_fullId(id);

	switch (type) {
	case T_SEMA_DI: {
		DI * p = pDIs[idx1_1 - 1];
		if (p)
			return p->bGrp;
	}	break;
	case T_SEMA_AI:{
		AI * p = pAIs[idx1_1 - 1];
		if (p)
			return p->bGrp;
	}
		break;
	case T_SEMA_DO:{
		DO * p = pDOs[idx1_1 - 1];
		if (p)
			return p->bGrp;
	}
		break;
	case T_SEMA_AO: {
		AO * p = pAOs[idx1_1 - 1];
		if (p)
			return p->bGrp;
	}
		break;
	}


	return false;
}
//-----------------------------------------------------------------------------------------
void Device::DO_Ctrl(const string & id, const string & val) {
	cmdItm itm;
	itm.cmdId = id;
	itm.val = val;
	itm.stat = STAT_SETPOINT_BUSY; // invalid data

	pthread_mutex_lock(&mutex_lock_cmdList);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "DO_Ctrl[+].";
#endif
//	cmds.clear();
	ClearVector(cmds);
	cmds.push_back(itm);
#if	DEBUG_SET_CTRL_CMD_LOCK
	cout << "DO_Ctrl[-]\n";
#endif
	pthread_mutex_unlock(&mutex_lock_cmdList);

}
void Device::devSetDIVal(const string &semId, u32 val) {
	u32 idx1_1 = getSemaphoreIdx(semId);
	DI * di = pDIs[idx1_1 - 1];
	if (di)
		di->devSetValHdl(semId, val);

}
//========================================================================================
void * DeviceDatHdlLoopThrd(void * param) {
	SET_THRD_NAME();
	vector<stDevAlmDB> devAlarms;
	DeviceManager * mgr = (DeviceManager *) param;
	sleep(2);
	while (mgr->runStat.isRunning()) {
		threadDebug
		mgr->setCommStat();
		mgr->DI_CheckLoop();
		mgr->semasCopy();
		mgr->doorMagneticLogic();
		mgr->cameraLogic();
		mgr->battsMeasDevSignalHdl();
		// ------
		mgr->filterAlarm();
//		devAlarms.clear();
		ClearVector(devAlarms);
		if (mgr->getReportAlms(devAlarms))
				DB.saveNewAlarmToUnConfirmedDB(devAlarms);
		sleep(1);
	}
	return ((void *) 0);
}
DeviceManager::DeviceManager() {
	pDevs = NULL;
}
DeviceManager::~DeviceManager() {
	quitNow();
	pthread_join(tid_loop, NULL);
	exit();
}
void DeviceManager::exit() {
	WHERE_AM_I
	ClearPointerVector(vecDevComm);
	for (u32 i = 0; i < maxDev; ++i) {
		if (pDevs[i] != NULL) {
			for (u32 j = 0; j < pDevs[i]->size(); ++j) {
				delete pDevs[i]->at(j);
			}
			pDevs[i]->clear();
//			ClearVector(pDevs[i]);
		}
	}
	delete pDevs;
}
#if 0
void DeviceManager::initDeviceSema() {

	u32 maxDevIdx;
	string devIds[100];
	u32 maxDevSema[100][4];
	for (u32 i = 0; i < 100; ++i) {
		maxDevSema[i][T_SEMA_DI] = 0;
		maxDevSema[i][T_SEMA_AI] = 0;
		maxDevSema[i][T_SEMA_DO] = 0;
		maxDevSema[i][T_SEMA_AO] = 0;
	}
//	u32 maxDevIdx = 0;
//	for (const auto & dev : gDat.vecDev) {
//		const char *s = dev.Id.c_str();
//		u32 devTypeId = (s[7] - '0') * 10 + (s[8] - '0');
//		char devId[3] = { s[7], s[8], 0 };
//		if (devTypeId > maxDevIdx) {
//			maxDevIdx = devTypeId;
//			devIds[devTypeId] = string(devId);
//		}
//	}
	for (const auto & sema : gDat.vecInitDI) {
		for (const auto & dev : gDat.vecDev) {
			const char *s = dev.Id.c_str();
			u32 devTypeId = (s[7] - '0') * 10 + (s[8] - '0');
			const char *ss = sema.id.c_str();
			u32 semDevTypeId = (ss[0] - '0') * 10 + (ss[1] - '0');
			if (devTypeId == semDevTypeId) {
				u32 semaIdx = (ss[3] - '0') * 10 + (ss[4] - '0');
				if (semaIdx > maxDevSema[devTypeId][0])
					maxDevSema[devTypeId][T_SEMA_DI] = semaIdx;
			}
		}
	}
	for (const auto & sema : gDat.vecInitAI) {
//		cout << "sema.id=" << sema.id << endl;
		for (const auto & dev : gDat.vecDev) {
//			cout << "\tdev.id=" << dev.Id << endl;
			const char *s = dev.Id.c_str();
			u32 devTypeId = (s[7] - '0') * 10 + (s[8] - '0');
			const char *ss = sema.id.c_str();
			u32 semDevTypeId = (ss[0] - '0') * 10 + (ss[1] - '0');
			if (devTypeId == semDevTypeId) {
				u32 semaIdx = (ss[3] - '0') * 10 + (ss[4] - '0');
//				cout << "\t\tdevTypeId=" << devTypeId << ",SemaIdx = " << semaIdx << endl;
				if (semaIdx > maxDevSema[devTypeId][1])
					maxDevSema[devTypeId][T_SEMA_AI] = semaIdx;
			}
		}
	}
	for (const auto & sema : gDat.vecInitDO) {
		for (const auto & dev : gDat.vecDev) {
			const char *s = dev.Id.c_str();
			u32 devTypeId = (s[7] - '0') * 10 + (s[8] - '0');
			const char *ss = sema.id.c_str();
			u32 semDevTypeId = (ss[0] - '0') * 10 + (ss[1] - '0');
			if (devTypeId == semDevTypeId) {
				u32 semaIdx = (ss[3] - '0') * 10 + (ss[4] - '0');
				if (semaIdx > maxDevSema[devTypeId][2])
					maxDevSema[devTypeId][T_SEMA_DO] = semaIdx;
			}
		}
	}
	for (const auto & sema : gDat.vecInitAO) {
		for (const auto & dev : gDat.vecDev) {
			const char *s = dev.Id.c_str();
			u32 devTypeId = (s[7] - '0') * 10 + (s[8] - '0');
			const char *ss = sema.id.c_str();
			u32 semDevTypeId = (ss[0] - '0') * 10 + (ss[1] - '0');
			if (devTypeId == semDevTypeId) {
				u32 semaIdx = (ss[3] - '0') * 10 + (ss[4] - '0');
				if (semaIdx > maxDevSema[devTypeId][3])
					maxDevSema[devTypeId][T_SEMA_AO] = semaIdx;
			}
		}
	}

#if DEBUG_INIT_DAT
	cout << "max dev = " << maxDev << endl;
#endif
	pDevSemaDefs = new devSemaDef[maxDev];
	pDevSemaDefs[0].typeIdx = 0; // 占位用
	for (u32 i = 1; i <= maxDevIdx; i++) {
		if ((maxDevSema[i][0] > 0) || (maxDevSema[i][1] > 0)
				|| (maxDevSema[i][2] > 0) || (maxDevSema[i][3] > 0)) {
			pDevSemaDefs[i].typeIdx = i;
			pDevSemaDefs[i].maxSema[T_SEMA_DI] = maxDevSema[i][T_SEMA_DI];
			pDevSemaDefs[i].maxSema[T_SEMA_AI] = maxDevSema[i][T_SEMA_AI];
			pDevSemaDefs[i].maxSema[T_SEMA_DO] = maxDevSema[i][T_SEMA_DO];
			pDevSemaDefs[i].maxSema[T_SEMA_AO] = maxDevSema[i][T_SEMA_AO];
//			pDevSemaDefs[i].grpSize_nxx = 0;
#if DEBUG_INIT_DAT
			cout << "maxSema[DI] = " << pDevSemaDefs[i].maxSema[T_SEMA_DI] << endl;
			cout << "maxSema[AI] = " << pDevSemaDefs[i].maxSema[T_SEMA_AI] << endl;
			cout << "maxSema[DO] = " << pDevSemaDefs[i].maxSema[T_SEMA_DO] << endl;
			cout << "maxSema[AO] = " << pDevSemaDefs[i].maxSema[T_SEMA_AO] << endl;
#endif
		} else
			pDevSemaDefs[i].typeIdx = 0; // empty
	}
	maxDev = maxDevIdx + 1;
}
void DeviceManager::initGrpInfo() {
//	for (const auto & it : gDat.vecInitGRP) {
//		const char *id = it.id.c_str();
//		const char *gId = it.grpId.c_str();
//		u32 devTypeId = (id[0] - '0') * 10 + (id[1] - '0');
//		if (0 == devTypeId)
//			continue;
//		// format is checked before.
//		u32 idx;
//		if (it.grpId.size() == 3) { // just nxx .
//			idx = (gId[1] - '0') * 10 + (gId[2] - '0');
//			if (idx > pDevSemaDefs[devTypeId].grpSize_nxx) // find max
//				pDevSemaDefs[devTypeId].grpSize_nxx = idx;
//		}
//	}
}
void DeviceManager::initNxx() {
	for (auto & git : gDat.vecInitGRP) {
#if DEBUG_INIT_DAT
		cout << git.devId << "," << git.semaId << "," << git.grpId << endl;
#endif
		if (git.grpId.size() != 3)  // just nxx .
			continue;
		if (git.devId.size() < 7) {
			for (const auto & dit : gDat.productDefCfgInitTbl) {
				if (dit.productModelIdx == git.devId) {
					Device * pDev = findDevice(gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + dit.Id);
					if (pDev) {
						pDev->initAddNxxItm(git.grpId, git.semaId);
					} else {
						cout << "can't find device of " << dit.Id << " in " << __FUNCTION__ << endl;
					}
				}
			}
		} else {
			Device * pDev = findDevice(gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + git.devId);
			if (pDev) {
				pDev->initAddNxxItm(git.grpId, git.semaId);
			} else {
				cout << "can't find device of " << git.devId << " in " << __FUNCTION__ << endl;
			}

		}
	}
}
void DeviceManager::initGrpSize() {
	for (auto & git : gDat.vecInitGRP) {
#if DEBUG_INIT_DAT
		cout << git.devId << "," << git.semaId << "," << git.grpId << endl;
#endif
		if (git.grpId.size() != 8)  // just fullId .
			continue;
		if (git.devId.size() < 7) {
			for (const auto & dit : gDat.devCommCfgTbl) {
				if (dit.productModelIdx == git.devId) {
					Device * pDev = findDevice(gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + dit.Id);
					if (pDev) {
						pDev->initAddGrpSizeItm(git.grpId, git.semaId);
					} else {
						cout << "can't find device of " << dit.Id << " in " << __FUNCTION__ << endl;
					}
				}
			}
		} else {
			Device * pDev = findDevice(gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + git.devId);
			if (pDev) {
				pDev->initAddGrpSizeItm(git.grpId, git.semaId);
			} else {
				cout << "can't find device of " << git.devId << " in " << __FUNCTION__ << endl;
			}

		}
	}
}
#endif
bool DeviceManager::initRelSemas() {
	string pre;
	switch (fsuMode) {
	case 1:	// 一体式
		pre = string("一体");
		break;
	case 2: // 分体式
		pre = string("分体");
		break;
	case 3:	// 室外机?
	default:
		cout << __FUNCTION__ << "wrong fsu mode !\n";
		return false;
	}
	string p = gDat.exePath + "cfg/" + pre + "扩展信号定义.csv";
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return false;

	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 5))
			return false;
		extSemaDefItm itm;
		itm.addr = atoi(col[0].c_str());
		if ((itm.addr <= 0) || (itm.addr > 8)) {
			cout << __FUNCTION__ << ":wrong ext board address[1~8] !\n";
			return false;
		}
		itm.ch = atoi(col[1].c_str());
		if ((itm.ch < 1) || (itm.ch >= 200)) {
			cout << __FUNCTION__ << ":wrong range of ch !\n";
			return false;
		}
		itm.name = col[2];
		itm.devId = col[3];
		if ((itm.devId.size() > 0) && !isIdAvailable(itm.devId, 7)) {
			cout << __FUNCTION__ << ":wrong devId !\n";
			return false;
		}
		itm.semaId = col[4];
		if ((itm.semaId.size() >0) && !isIdAvailable(itm.semaId, 8)) {
			cout << __FUNCTION__ << "wrong semaId !\n";
			return false;
		}
		if (fsuMode == 2) {
			if (getDevType(itm.devId) == 17) {
				const char *c = itm.semaId.c_str();
				if(    (c[0] == '1')
					&& (c[1] == '7')
					&& (c[2] == '0')
					&& (c[3] == '0')
					&& (c[4] == '5') ) {
					int ch = itm.ch % 100;
					gDat.doorMagDefs.push_back(make_pair(itm.addr, ch));
				}
			}
		} else if (fsuMode == 1) {
			if (getDevType(itm.devId) == 18) {
				const char *c = itm.semaId.c_str();
				if(    (c[0] == '1')
					&& (c[1] == '8')
					&& (c[2] == '0')
					&& (c[3] == '0')
					&& (c[4] == '3') ) {
					int ch = itm.ch % 100;
					gDat.infraDef = make_pair(itm.addr, ch);
				}
			}
		}
		gDat.extSemaDefTbl.push_back(itm);
	}
	return true;
}
bool DeviceManager::init_alarm_filter() {
	// 相关设备过滤准备
	vector<vector<string> > vecDevs;
	vecDevs.clear();
	string path = gDat.exePath + string("cfg/AcDistribution.conf");
	if (!readCsv(vecDevs, path))
		return false;
	for (u32 x = 0; x < vecDevs.size(); x++) {
		if (vecDevs[x].size() == 0)
			continue;
		AlmFilter * pFilter = new AlmFilter();
#if DEBUG
		cout << "\nAcDistribution:";
		for (auto &d : vecDevs[x])
			cout << d << "  ";
		cout << endl << endl;
#endif
		if (pFilter->init(vecDevs[x]))
			filters.push_back(pFilter);
		else
			return false;
	}
	return true;
}
bool DeviceManager::init_local_device() {
	pDevs = new vector<Device*>*[maxDev];
	for (u32 i = 0; i < maxDev; ++i)
		pDevs[i] = NULL;

	string model_07, model_17, model_19;
	for (auto & m : gDat.productDefCfgInitTbl) {
		u32 type = atoi(m.devType.c_str());
		switch(type) {
		case 7:
			model_07 = m.productModelIdx;
			break;
		case 19:
			model_19 = m.productModelIdx;
			break;
		case 17:
			if (fsuMode == 2) {
				if (m.productModel_name == "分体门禁系统")
					model_17 = m.productModelIdx;
			}
		}
	}
	for (auto & i : gDat.vecDev) {
		Device * p = new Device();
		u32 typeIdx = getDeviceTypeVal(i.Id);
		bool got = false;
		string model;
		switch (typeIdx) {
		case DEV_TYPE_07_BattMeas:
			vecBattDevs.push_back(p);
			model = model_07;
			got = true;
			break;
		case DEV_TYPE_19_TongXinZhongDuan:
			got = true;
			model = model_19;
			pDev_19 = p;
			break;
		case DEV_TYPE_17_DoorGuard:
			got = true;
			model = model_17;
			pDev_17 = p;
			break;
		}
		if (got)
			setNewDevice(p, i.Id, model);
	}
	if (!initDev18s())
		return false;
//	if (!initRelDevs())
//		return false;

	if (!initRelSemas())
		return false;

	// 相关设备过滤准备
	if (!init_alarm_filter())
		return false;

	return true;
}
bool DeviceManager::initDevComm(DevComm * p,
		const portConfigItm & cfgItm) {
	if (!p->init(cfgItm.ttyType, cfgItm.ttyPort, cfgItm.devId)) {
		cout << "comm init error !\n";
		return false;
	}
	gDat.vecDevComm.push_back(p);
	cout << "set comm port : " << cfgItm.ttyType << cfgItm.ttyPort << " - OK.\n";
	Device * dev = new Device; // 占坑用
	const char * s = cfgItm.devId.c_str();
	if ((*s == '1') && (*(s + 1) == '7'))
		pDev_17 = dev;
	dev->setCommPort(p);

	dev->setReady(false);
	p->pDev = dev;
	setEmptyDevice(dev,
			gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + cfgItm.devId);
	return true;
}
bool DeviceManager::initDev18s() {
	u8 dev18Def = gDat.gatherersInstallDat8;
	u8 mask = 1;
	for (u32 i = 0; i < 8; ++i) {
		mask = (1 << i);
		if ((dev18Def & mask) > 0) {
			portConfigItm itm;
			itm.ttyType = "ttyE";
			char ch[3] = {0, '0', 0};
			ch[0] = i + '0';
			itm.ttyPort = ch;
			char devId[8] = {'1','8','0','0','0','0','0', 0};
			devId[6] = ch[0] + 1;
			itm.devId =devId;
#if 1//DEBUG_INIT_DAT
			cout << "init dev18:" << itm.ttyType << ",";
			cout << itm.ttyPort << "," << itm.devId << endl;
#endif
			gDat.portCfgTbl_dev18.push_back(itm);
		}
	}
	for (const auto & i : gDat.portCfgTbl_dev18) {
		if (!initPort_Dev18s(i))
			return false;
	}
	return true;
}
bool DeviceManager::initPort_Dev18s(const portConfigItm & cfgItm) {
	DevComm *p = new DevComm;
	p->scanable = false;
	p->setOnScan(false);
	bool findModel = false;
	bool findPPP = false;
	for(const auto & model : gDat.productDefCfgInitTbl) {
		if (18 == atoi(model.devType.c_str())) {
			findModel = true;
			DevComm::scanDat sd;
			sd.defCfg = &model;
			string name = model.productModel_name;
#if DEBUG_INIT_DAT
			cout << name << "\t";
#endif
			for (const auto & pp : gDat.vecPPP) {
				if (pp->fmt.type == model.format) {
#if DEBUG_INIT_DAT
					cout << pp->so << endl;
#endif
//						cout << pp->fmt.type << "," << model.format << endl;
					findPPP = true;
					sd.ppp = pp;
					p->scanTbl.push_back(sd);
					break;
				}
			}
			if (!findPPP)
				break;
		}
	}
	if (!findModel || !findPPP) {
		cout << "scan table init error !" << endl;
		return false;
	}
	p->setCurrDevDat(&(p->scanTbl.at(0)));
	if (!initDevComm(p, cfgItm)) {	// create Device * pDev;
		delete p;
		return false;
	}
	string fullDevId = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + cfgItm.devId;
	setEmptyDevice(p->pDev,fullDevId);
	p->pDev->setReady(false);

	const char *s = cfgItm.ttyPort.c_str();
	char idx = *s - '0';
	char ch[3] = {'0',0,0};
	ch[1] = '1' + idx;
	string pre;
	switch (fsuMode) {
	case 1:	// 一体式
		pre = string("一体");
		break;
	case 2: // 分体式
		pre = string("分体");
		break;
	case 3:	// 室外机?
	default:
//		cout << __FUNCTION__ << "wrong fsu mode !\n";
		return false;
	}
	string productModel_name = pre + "机房环境" + string(ch);
	p->pDev->init(fullDevId, productModel_name);

	if (!p->initPort()) {
		delete p;
		return false;
	}

	for (auto & i : gDat.prtclDatTbl) {
		string fileName = findFileNameOfPath(i->devName);
//		cout << "looking ... " << fileName << endl;
		if (fileName == productModel_name) {
			p->setPrtcl(i);
			break;
		}
	}
	if (!p->initWorkThread()) {
		delete p;
		return false;
	}
	vecDevComm.push_back(p);
	return true;
}
bool DeviceManager::initPort() {
	for (const auto & cfgItm : gDat.portCfgTbl) {
		bool find = false;
		const char * s = cfgItm.devId.c_str();
		if ((*s == '1') && (*(s + 1) == '8')) {	// dev_18 不用扫描
		} else {
			for (const auto & scanItm : gDat.portDevScanTbl) {
				string id = cfgItm.devId;
				u32 sz = id.size();
				if (sz == 6)
					id = "0" + id;
				string out;
				getDevTypeStr(id, out);
				int dt = atoi(out.c_str());
				if (dt == scanItm.devType) {
					DevComm *p = new DevComm;
					p->scanable = true;
					if (!(p->initScanTbl(scanItm))) {	// 解析文件等初始化
						cout << "comm dev init scan table error !" << endl;
						delete p;
						return false;
					}
					find = true;
					if (!initDevComm(p, cfgItm)) {
						delete p;
						return false;
					}
					if (!p->initWorkThread()) {
						delete p;
						return false;
					}
					vecDevComm.push_back(p);
					break;
				}
			}
			if (!find) {
				cout << "can't find device type in \'DevScan.csv\' file!\n";
				return false;
			}
		}
	}
	return true;
}
//-------------------------------------------------------
void DeviceManager::DO_LightCtrl(Device * pDev_18, bool OnOff) {
//#if ENV_DEV
#if 1//DEBUG_LIGHT_CTRL
	string onoff = OnOff ? string("On") : string("Off");
	cout << "==================" << endl;
	cout << "  [^^^^] " << onoff << endl;
	cout << "==================" << endl;
#endif
	if (pDev_18 != NULL)
		pDev_18->DO_Ctrl(OnOff ? string("l01") : string ("l02"), string());
//#endif
}
void DeviceManager::semasCopy() {
	for (const auto & i : gDat.extSemaDefTbl) {
		string fullDevId = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + i.devId;
		Device * pObjDev = findDevice(fullDevId);
//		if (fullDevId == "13100140700002")
//			cout << "now !\n";
		u32 idx = i.ch % 100 - 1;
		if (pObjDev /*&& pObjDev->isReady()*/) {
			if (i.ch < 100) {
				u8 out;
				if (allExtDat[i.addr - 1].getDIVal(idx, out))
					pObjDev->setDIStatById(i.semaId, out);
			} else if ((i.ch > 100) && (i.ch < 200)) {
				string val = allExtDat[i.addr - 1].getAIStr(idx);
//				cout << "val = " << val << endl;
				if (!val.empty())
					pObjDev->setAIValStrById(i.semaId, allExtDat[i.addr - 1].getAIStr(idx));
			}
		}
	}
}
bool DeviceManager::cameraSignalInit() {
	string devId_env = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + "1800001";
	Device * pDev_18 = findDevice(devId_env);
	if (!pDev_18 || !pDev_17)
		return false;
	gDat.cameraDat.cameraDevId = devId_env;
	gDat.cameraDat.pDev_18 = pDev_18;
	gDat.cameraDat.pDev_17 = pDev_17;
	DI * pDI_Infr 			= pDev_18->getDI(getSemaphoreIdx(cameraRelDevs::id_Infr));
	DI * pDI_doorMagnetic 	= pDev_17->getDI(getSemaphoreIdx(cameraRelDevs::id_doorMagnetic));
	DI * pDI_doorLock 		= pDev_17->getDI(getSemaphoreIdx(cameraRelDevs::id_doorLock));
	DI * pDI_IllegalEntry	= pDev_17->getDI(getSemaphoreIdx(cameraRelDevs::id_IllegalEntry));

	if (!pDI_Infr || !pDI_doorMagnetic || !pDI_doorLock) {
		cout << "no related signal !" << endl;
		return false;
	}
	gDat.cameraDat.pDI_Infr = pDI_Infr;
	gDat.cameraDat.pDI_doorLock = pDI_doorLock;
	gDat.cameraDat.pDI_doorMagnetic = pDI_doorMagnetic;
	gDat.cameraDat.pDI_IllegalEntry = pDI_IllegalEntry;

	return true;
}
void DeviceManager::battsMeasDevSignalHdl() {
	for (const auto & it : vecBattDevs)
		battMeasDevSignalHdl(it);
}
void DeviceManager::battMeasDevSignalHdl(Device * pDev_07) {
	float firstHalfVolt, secondHalfVolt;
	if (pDev_07->getAIValById("07106001", firstHalfVolt) &&
	pDev_07->getAIValById("07107001", secondHalfVolt)) {
		float totalVolt = firstHalfVolt + secondHalfVolt;
		pDev_07->setAIValById("07102001", totalVolt);
		string totalStr = floatToString(totalVolt, 3, 2);
#if DEBUG_BATT_DAT
		cout << "total batt volt = " << totalStr << "V" << endl;
#endif
		float halfTotal = totalVolt / 2;
		float balance = (halfTotal - firstHalfVolt) * 100 / totalVolt;
#if DEBUG_BATT_DAT
		cout << "balence = " << balance << endl;
#endif
		if (balance < 0)
			balance = 0 - balance;
		u32 ImbalanceStr = (balance > 5) ? 1 : 0;
//		pDev_07->setDIStatById("07005001", ImbalanceStr);
		DI * p = pDev_07->getDI("07005001");
		if (p)
			p->fsuCalcReverse(/*pDev->fullId, */0, ImbalanceStr, balance);
	}
// XXX batt total current !!!
//	float fBattNum;
//	if (!st.pDev_06->getAIValById("06126001", fBattNum))
//		return;
//	u32 battNum = (u32)fBattNum;
//	float battI = 0;
//	for (u32 i = 0; i < battNum; ++i) {
//		float I = 0;
//		string sid = semaIdCalcAdd("06115001", i);
//		if (!st.pDev_06->getAIValById(sid, I))
//			continue;
//		battI += I;
//	}
//	pDev_07->setAIValById("07104001", battI);
//#if DEBUG_BATT_DAT
//	cout << "total bat I = " << battIStr << "A" << endl;
//#endif
}
void DeviceManager::cameraLogic() {
	switch (fsuMode) {
	case 1:	// 一体式
		if (cameraSignalInit())
			chkCameraLogic(gDat.cameraDat);
		break;
	case 2: // 分体式, 无摄像头
		break;
	case 3:	// 室外机?
	default:
//		cout << __FUNCTION__ << "wrong fsu mode !\n";
		return ;
	}
}
void DeviceManager::chkCameraLogic(cameraRelDevs & st) {
	u8 currValDM;
	if (!st.pDI_doorMagnetic->getVal(0, currValDM))			// 门磁
		return;
	u8 currValDL;
	if (!st.pDI_doorLock->getVal(0, currValDL))				// 门锁
		return;
	u8 currValInfra;
	if (!st.pDI_Infr->getVal(0, currValInfra))				// 红外
		return;
 	bool bCurrStatDM = currValDM == 1;
	bool bCurrStatDL = currValDL == 0;
	bool bCurrStatIllegalEntry = st.bLastStatIllegalEntry;
	bool cond_illegalEntry_dev;
	bool cond_illegalEntry_fsu;
	bool bAvail_illegalEntry = false;
	if (st.pDI_IllegalEntry != NULL) {
		u8 currIllegalEntry;
		bAvail_illegalEntry = st.pDI_IllegalEntry->getVal(0, currIllegalEntry);
		if (bAvail_illegalEntry)
			cond_illegalEntry_dev = currIllegalEntry == 1;
	}

	bool event_openDoor = !st.bLastStatDM && bCurrStatDM;				// 门刚被打开
	bool event_doorLock = st.bLastStatDL && bCurrStatDL;				// 门锁还没打开
	if (bAvail_illegalEntry) {
		bCurrStatIllegalEntry = cond_illegalEntry_dev;
	} else {
		bool cond_illegalEntry = (event_openDoor && event_doorLock);		// 非法开门！

		if (cond_illegalEntry)
			bCurrStatIllegalEntry = true;
		if (!bCurrStatDL)												// 门锁没锁
			bCurrStatIllegalEntry = false;
		cond_illegalEntry_fsu = bCurrStatIllegalEntry;
	}
	bool bCurrStatInfra = currValInfra == 1;
	bool event_infraAlmAct = !st.bLastStatInfra && bCurrStatInfra;		// 红外信号刚产生
	//bool event_infraAlmDisAct = st.bLastStatInfra && !bCurrStatInfra;	// 红外信号刚消失

	st.pDI_Infr->setFilter(!bCurrStatIllegalEntry);

	bool ctrl_TakePhoto = false;
	bool bCurrStatlight = false;

	if (event_openDoor)
		ctrl_TakePhoto = true;					// 刚开门设可拍照

	if (bCurrStatDM)	// 开着门就开灯
		bCurrStatlight = true;

	if (!bCurrStatDM) {	// 关门状态，根据红外决定灯状态和是否拍照
		bCurrStatlight = bCurrStatInfra;
		ctrl_TakePhoto = event_infraAlmAct;
	}

	if (ctrl_TakePhoto)
		notifyCamera();	// 通知拍照

	bool ctrl_turnOnLight = !st.bLastStatLight && bCurrStatlight;	// 动作
	bool ctrl_turnOffLight = st.bLastStatLight && !bCurrStatlight;	// 动作

	if (ctrl_turnOnLight)
		DO_LightCtrl(st.pDev_18, true);

	if (ctrl_turnOffLight)
		DO_LightCtrl(st.pDev_18, false);

#if 1//DEBUG_LIGHT_CTRL
	cout << endl;
	cout << "\t\t\t\t doorMagnetic:      \t " << st.bLastStatDM << "," << bCurrStatDM << endl;
	cout << "\t\t\t\t doorLock:          \t " << st.bLastStatDL << "," << bCurrStatDL << endl;
	if (bAvail_illegalEntry) {
		cout << "\t\t\t\t cond_illegalEntry_dev:\t " << cond_illegalEntry_dev << endl;
		cout << "\t\t\t\t cond_illegalEntry_fsu:\t " << " " << endl;
	} else {
		cout << "\t\t\t\t cond_illegalEntry_dev:\t " << " " << endl;
		cout << "\t\t\t\t cond_illegalEntry_fsu:\t " << cond_illegalEntry_fsu << endl;
	}
	cout << "\t\t\t\t take photo:        \t " << ctrl_TakePhoto << endl;
	cout << "\t\t\t\t light ctrl:        \t " << st.bLastStatLight << "," << bCurrStatlight <<  endl;
	cout << "\t\t\t\t infra:             \t " << st.bLastStatInfra << "," << bCurrStatInfra << endl;
	cout << endl;
#endif

	st.bLastStatDM = bCurrStatDM;
	st.bLastStatDL = bCurrStatDL;
	st.bLastStatInfra = bCurrStatInfra;
	st.bLastStatLight = bCurrStatlight;
	st.bLastStatIllegalEntry = bCurrStatIllegalEntry;

}
void DeviceManager::doorMagneticLogic() {
	switch (fsuMode) {
	case 1:	// 一体式
		break;
	case 2: // 分体式
		// any 1 => almOn, all 0 => almOff
		{
			if(pDev_17 != NULL) {
				pDev_17->setReady(true);

				bool almOn = false;
				for(const auto &i : gDat.doorMagDefs) {
					u8 out;
					allExtDat[i.first - 1].getDIVal(i.second - 1, out);
					if (out) {
						almOn = true;
						break;
					}
				}
				u32 stat = almOn ? 1 : 0;
				pDev_17->setDIStatById("17005001", stat);
			}
		}
		break;
	case 3:	// 室外机?
	default:
		return ;
	}
}
//-----------------------------------------------------------------------------------------------------
Device * DeviceManager::findDevice(string fullDevId) {
	if (fullDevId.size() == 0)
		return NULL;
	const char * devId = fullDevId.c_str();
	u32 typeIdx = (devId[7] - '0') * 10 + (devId[8] - '0');
	if (typeIdx >= maxDev)
		return NULL;
	if (pDevs[typeIdx] == NULL)
		return NULL;
	size_t sz = pDevs[typeIdx]->size();
	for (u32 i = 0; i < sz; ++i) {
//		cout << pDevs[typeIdx]->at(i)->getFullId() << "!!!\n";
		if (fullDevId == pDevs[typeIdx]->at(i)->getFullId()) {
			return pDevs[typeIdx]->at(i);
		}
	}
	return NULL;
}
void DeviceManager::setEmptyDevice(Device * pDev, const string & fullDevId) {
	u32 typeIdx = getDeviceTypeVal(fullDevId);
	if (pDevs[typeIdx] == NULL)
		pDevs[typeIdx] = new vector<Device*>;
	pDevs[typeIdx]->push_back(pDev);
}
void DeviceManager::changeTheDevice(Device * pDev, const string & fullDevId, string & newModelIdx) {
	pDev->setReady(false);
	bool find = false;
	for (auto & d : gDat.productDefCfgInitTbl) {
		if (d.productModelIdx == newModelIdx) {
			pDev->init(fullDevId, d.productModel_name);
			find = true;
			break;
		}
	}
	if (!find)
		cout << "\n can't find device in prodectDefCfgInitTbl\n\n";
}
void DeviceManager::setNewDevice(Device * pDev, const string & fullDevId, string & model) {
#if DEBUG
	cout << "make the device --> " << fullDevId << endl;
#endif
	u32 typeIdx = getDeviceTypeVal(fullDevId);
	if (pDevs[typeIdx] == NULL)
		pDevs[typeIdx] = new vector<Device*>;
	string devModelName;
	for (auto & d : gDat.productDefCfgInitTbl) {
		if (d.productModelIdx == model) {
			devModelName = d.productModel_name;
			break;
		}
	}
	pDevs[typeIdx]->push_back(pDev);
	pDev->init(fullDevId, devModelName);
}
void DeviceManager::getAllDevices(vector<string> & ids) {
//	ids.clear();
	ClearVector(ids);
	for (const auto & dev : gDat.vecDev) {
		ids.push_back(dev.Id);
	}
}
void DeviceManager::getTheDeviceAllIds(string fullDevId, vector<string> & ids) {
//	ids.clear();
	ClearVector(ids);
	if (getDeviceTypeVal(fullDevId) == 18) {
		const char * s = fullDevId.c_str();
		string shortDevId = string(s + 7);
		for (const auto & i : gDat.dev18CfgTbl) {
			if (shortDevId == i.sc_devId) {
//				u8 addr = i.addr;
				ids.push_back(i.semaId);
			}
		}
	} else {
		Device * pDev = findDevice(fullDevId);
		if (pDev)
			pDev->getAllIds(ids);
	}
}
//----------------------------------------------------------------------------------------------------
string DeviceManager::addr2DeviceShortId(u8 addr) {
	string dev18Id = "180000";
	char ch[2] = {0};
	ch[0] = '0' + addr - 1;
	dev18Id += ch;
	return dev18Id;

}
string DeviceManager::changeDeviceFullId(const string & fullDevId) {
	if (getDeviceTypeVal(fullDevId) == 18) {
		const char * s = fullDevId.c_str();
		string shortDevId = string(s + 7);
		for (const auto & i : gDat.dev18CfgTbl) {
			if (shortDevId == i.sc_devId)
				return gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + addr2DeviceShortId(i.addr);
		}
	} else
		return fullDevId;
	return gDat.emptyStr;
}
string DeviceManager::changeDeviceFullId_r(const string & fullDevId, const string & semaId) {
	const char * s = fullDevId.c_str();
	string shortId =  string(s + 7);
	return gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + changeDeviceShortId_r(shortId, semaId);
}
string DeviceManager::changeDeviceShortId(const string & shortDevId, const string & semaId) {
	string devType;
	getDevTypeStr(shortDevId, devType);
	if(devType == "18") {
		for (const auto & i : gDat.dev18CfgTbl) {
			if ((shortDevId == i.sc_devId) && (semaId == i.semaId))
				return addr2DeviceShortId(i.addr);
		}

	} else {
		return shortDevId;
	}
	return gDat.emptyStr;
}
string DeviceManager::changeDeviceShortId_r(const string & shortDevId, const string & semaId) {
	string devType;
	getDevTypeStr(shortDevId, devType);
	if(devType == "18") {
		string shortSId = semaId;
		if (semaId.size() == 10)
			shortSId = getSemaphoreShortId(semaId);
		const char * s = shortDevId.c_str();
		u8 addr =  *(s + 6) - '0' + 1;
		for (const auto & i : gDat.dev18CfgTbl) {
			if ((addr == i.addr) && (shortSId == i.semaId)) {
				return i.sc_devId;
			}
		}

	} else {
		return shortDevId;
	}
	return gDat.emptyStr;
}
//------------------------------------------------------------------------------------------------------
void DeviceManager::getTheDeviceSemaphoes_current(const string & fullDevId,
		vector<stSemaphore_l> & inOut) {
	Device * pDev = findDevice(changeDeviceFullId(fullDevId));
	if (pDev != NULL)
		pDev->getSemaphoes_current(inOut);

}
void DeviceManager::getTheDeviceSemaphoes_history(const string & fullDevId,
		vector<string> & inIds, vector<stSemaphore> & out, string startTime,
		string endTime) {
	Device * pDev = findDevice(changeDeviceFullId(fullDevId));
	if (pDev != NULL)
		pDev->getSemaphoes_history(inIds, out, startTime, endTime);
}
void DeviceManager::upDateHistoryDat() {
	if (runStat.isRunning()) {
		for (u32 i = 0; i < maxDev; ++i) {
			if (pDevs[i] != NULL) {
				for (u32 j = 0; j < pDevs[i]->size(); ++j) {
					pDevs[i]->at(j)->upDateHistoryDat();
				}
			}
		}
	}
}
void * DeviceManager::setPointProc(void * p) {
	SET_THRD_NAME();
	DeviceManager * mgr = (DeviceManager *) p;

	for (const auto & dev : mgr->setPointDat.devs) {
		Device * pDev = mgr->findDevice(dev.devId);
		if (pDev != NULL) {
			pDev->setPoint(dev.sems);
		}
	}
	for (;;) {
		cout << __FUNCTION__ << "1\n";
		bool allDone = true;
		for (const auto & dev : mgr->setPointDat.devs) {
			cout << __FUNCTION__ << "2\n";
			Device * pDev = mgr->findDevice(dev.devId);
			if ((pDev != NULL) && (pDev->isReady())) {
				cout << __FUNCTION__ << "21\n";
				if (!pDev->isTaskEnd()) {
					allDone = false;
					break;
				}
			} else {
				cout << __FUNCTION__ << "22\n";
				setPointReponseOfDevice result;
				result.devId = dev.devId;
				for (const auto & sema : dev.sems)
					result.failedIds.push_back(sema.id);
				mgr->setPointResult.push_back(result);
			}
		}
		if (!allDone) {
			cout << __FUNCTION__ << "31\n";
			sleep(1);
		}
		else {
			cout << __FUNCTION__ << "32\n";
//			mgr->setPointResult.clear();
			ClearVector(mgr->setPointResult);
			for (const auto & dev : mgr->setPointDat.devs) {
				cout << __FUNCTION__ << "321\n";
				Device * pDev = mgr->findDevice(dev.devId);
				if (pDev != NULL) {
					cout << __FUNCTION__ << "3211\n";
					setPointReponseOfDevice result;
					result.devId = dev.devId;
					vector<cmdItm> _rslt;
					pDev->getSetPointResult(_rslt);
					for (const auto r : _rslt) {
						if (r.stat == STAT_SETPOINT_OK)
							result.successIds.push_back(
									/*gDat.semaIdPreString + */r.cmdId);
						else
							result.failedIds.push_back(
									/*gDat.semaIdPreString + */r.cmdId);
					}
					mgr->setPointResult.push_back(result);
				}
			}
			return (void *) 0;
		}
	}
	return (void *) 0;
}
void DeviceManager::setPoint(Devs_Semaphores & in,
	vector<setPointReponseOfDevice> & devResult) {
	setPointDat = in;
//	devResult.clear();
	ClearVector(devResult);
	pthread_t id;
	int ret = pthread_create(&id, NULL, DeviceManager::setPointProc, this);
	if (ret < 0)
		return;
	pthread_join(id, NULL);

	devResult = setPointResult;
}
void DeviceManager::getTheDeviceThreshold(string fullDevId,
		vector<stThreshold> & inOut) {

	if (getDeviceTypeVal(fullDevId) == 18) {
		const char * s = fullDevId.c_str();
		string shortDevId = string(s + 7);
		for (const auto & i : gDat.dev18CfgTbl) {
			if (shortDevId == i.sc_devId) {
				u8 addr = i.addr;
				string dev18Id = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + "180000";
				char ch[2] = {0};
				ch[0] = '0' + addr;
				dev18Id += ch;
				Device * pDev = findDevice(dev18Id);
				if (pDev != NULL)
					pDev->getThreshold(inOut);
				break;
			}
		}
	} else {
		Device * pDev = findDevice(fullDevId);
		if (pDev != NULL)
			pDev->getThreshold(inOut);
	}


}
vector<thrdItm> rst;
void wrtThresholdProc() {
	string file = gDat.exePath + string("cfg/init_list.csv");

	vector<vector<string> > oldDat;
	if (!readCsv(oldDat, file)) {
		cout << "read csv error at " << __FUNCTION__ << endl;
		log("read csv error at wrtThresholdProc()\n");
		return;
	}
	//cout << "read end." << endl;

//	bool save = false;
	for (const auto & thrd : rst) {
		bool find = false;
		string newDevId = DevMgr.changeDeviceShortId_r(thrd.devId, thrd.semaId);
		for (auto & line : oldDat) {
			if (line.size() > 3) {
				string sid = line.at(1);
//				if (sid.size() == 7)
//					sid = string("0") + sid;
				if ((line.at(0) == newDevId) && (sid == getSemaphoreShortId(thrd.semaId))) {
					line.at(2) = thrd.thrd;
#if DEBUG_DAT_DETAIL
					cout << "save threshold (" << thrd.devId << "," << thrd.semaId << "," << thrd.thrd << ")" << endl;
#endif
					find = true;
//					save = true;
					break;
				}
			}
		}
		if (!find) {
			vector<string> tmp;
			tmp.push_back(newDevId);
			tmp.push_back(getSemaphoreShortId(thrd.semaId));
			tmp.push_back(thrd.thrd);
			tmp.push_back(thrd.name);
			oldDat.push_back(tmp);
		}
#if DEBUG_DAT_DETAIL
		if (!find) {
			cout << "couldn't find the threshold itm, devid(" <<thrd.devId << ")," ;
			cout << "semaId(" << thrd.semaId << ")" << endl;
		}
#endif
	}
//	if(save)
		writeCsv(file, oldDat);
}
void DeviceManager::setThreshold(ReqThresholds & in,
		vector<setThresholdReponseOfDevice> & devResult) {
//	devResult.clear();
	ClearVector(devResult);
//	rst.clear();
	ClearVector(rst);

	for (const auto & dev : in.devs) {
		Device * pDev = findDevice(changeDeviceFullId(dev.devId));
		setThresholdReponseOfDevice result;
		if ((pDev != NULL)  && (pDev->isReady())) {
			pDev->setThreshold(dev.thrs, result);
			devResult.push_back(result);
		} else {
			result.devId = dev.devId;
			result.vecSuccessItms.clear();
			result.failedIds.clear();
			for (const auto & thrs : dev.thrs)
				result.failedIds.push_back(thrs.id);
			devResult.push_back(result);
		}
	}
	for (const auto & dev : devResult) {
		for (const auto & it : dev.vecSuccessItms) {
			rst.push_back(thrdItm(it.devId, it.semaId, it.thrd, it.name));
		}
	}
	std::thread t(wrtThresholdProc);
//	t.join();
	t.detach();
}
//--------------------------------------------------------------------------------------------------------
void DeviceManager::getAIUnitById(string &fullId, string &u) {
	if (fullId.size() == 0)
		return;
	const char * sId = fullId.c_str();
	u32 devTypeIdx = (sId[0] - '0') * 10 + (sId[1] - '0');
	if (sId[2] != '1') {
		u = string();
		return;
	}
	u32 semaType = (sId[3] - '0') * 10 + (sId[4] - '0');
	if (devTypeIdx >= maxDev) {
		u = string();
		return;
	}
	Device * pDev = pDevs[devTypeIdx]->at(0);
	if (pDev)
		return pDev->getAIUnitById(semaType - 1, u);
	u = string();
}
bool DeviceManager::run() {
	int ret = pthread_create(&tid_loop, NULL, DeviceDatHdlLoopThrd, this);
	if (ret < 0) {
		cout << "thread 'FsuCheckProc' create error !" << endl;
		return false;
	}
	startNow();
	return true;
}
void DeviceManager::checkGathCommStat(unsigned char bits) {
	Device * p = pDev_19;
	if (!p) return;
	unsigned char mask = gDat.gatherersInstallDat8;
	bool b = (mask != bits);	// some gatherer comm error!
#if DEBUG_485_HEART
	u32 i = bits;
	cout << "\t\tGatherer stat is " << i << endl;
#endif
	//19004001
	p->devSetDIVal(string("19004001"), b ? 1 : 0);
}
void DeviceManager::setCommStat() {	// 通信状态有变化就更新相应DI值
	Device * p = pDev_19;
	if (!p) return;

//	cout << "\n\tsetCommStat() \n" << endl;
	u32 val = 0;
	for (u32 i = 0; i < maxDev; ++i) {
		if (pDevs[i] != NULL) {
			bool commErr = false;
			for (u32 j = 0; j < pDevs[i]->size(); ++j) {
				Device * pD = pDevs[i]->at(j);
				if (pD != NULL) {
					for (auto & i : gDat.vecDevCommErrCfg) {
						if (i.second == pD->devTypeId) {
//							cout << "pD->id=" << pD->id << "|";
							if (pD->pComm != NULL)
								val = (pD->pComm->isCommErr()) ? 1 : 0;
							else
								val = 0;
							if(val == 1)
								commErr = true;
							p->devSetDIVal(i.first, val);
							break;
						}
					}
				}
				if (commErr) // 同类设备一个告警即可
					break;
			}
		}
	}
}
void DeviceManager::filterAlarm() {
	for (auto &it : filters) {
		it->chkAndFilterAlarm();
	}
}
void DeviceManager::DI_CheckLoop() {
	for (u32 i = 0; i < maxDev; ++i) {
		if (pDevs[i] != NULL) {
			for (u32 j = 0; j < pDevs[i]->size(); ++j) {
				pDevs[i]->at(j)->checkDIs();
			}
		}
	}
}
bool DeviceManager::getReportAlms(vector<stDevAlmDB> &v) {//vector<BInt::stAlarmDB> & vec) {
	bool rtn = false;
	for (u32 i = 0; i < maxDev; ++i) {
		if (pDevs[i] != NULL) {
			for (u32 j = 0; j < pDevs[i]->size(); ++j) {
				Device * d = pDevs[i]->at(j);
				if (!d->isReady())
					continue;
				stDevAlmDB st;
				st.devId = d->fullId;
				if (d->getAlm(st.v))
					rtn = true;
				v.push_back(st);
			}
		}
	}
	return rtn;
}
#if 0
//bool DeviceManager::battsMeasDevSignalInit() {
//	if (vecBattRelDevs.empty()) {
//		vector<string> ids;
//		ids.clear();
//		getDevsIdsByteType("07", ids);// 多组
//		for (const auto & id : ids) {
//			if (!battMeasDevSignalInit(id)){
//				vecBattRelDevs.clear();	// 必须全部有效，否则清空
//				return false;
//			}
//		}
//	}
//	return true;
//}
//const string& DeviceManager::getRelDevId(const string & in, const string & refType) {
//	for (const auto & pr : vecDevRelPair) {
//		if (pr.first == in) {
//			string ty;
//			getDevTypeStr(pr.second, ty);
//			if (ty == refType) {
//				return pr.second;
//			}
//		}
//	}
//	return gDat.emptyStr;
//}
//bool DeviceManager::battMeasDevSignalInit(const string & devId_batt) {
//	Device * pDev_07 = findDevice(devId_batt);
//	Device * pDev_06 = findDevice(getRelDevId(devId_batt, "06"));
//	Device * pDev_18 = findDevice(getRelDevId(devId_batt, "18"));
//
//	if (!pDev_18 || !pDev_07 || !pDev_06)
//		return false;
//	battRelDevs st;
//	st.battDevId = devId_batt;
//	st.pDev_06 = pDev_06;
//	st.pDev_07 = pDev_07;
//	st.pDev_18 = pDev_18;
//
//	vecBattRelDevs.push_back(st);
//	return true;
//}
//void DeviceManager::copyAIVal(Device * sd, const string &s, Device * od, const string &o) {
//	float tmp;
//	if (sd->getAIValById(s, tmp))
//		od->setAIValById(o, tmp);
//}
//void DeviceManager::upDateADeviceSemaphoes(string fullDevId,
//		vector<stIdVal> & in) {
//	Device * pDev = findDevice(fullDevId);
//	if (pDev != NULL)
//		pDev->upDateSemaphoes(in);
//}
//void DeviceManager::chkCamerasLogic() {
//	if (camerasSignalInit()) {
//		for (auto & it : vecCameraRelDevs)
//			chkCameraLogic(it);
//	}
//}
#endif
