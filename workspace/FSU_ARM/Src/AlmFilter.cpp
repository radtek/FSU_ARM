/*
 * AlmFilter.cpp
 *
 *  Created on: 2016年10月16日
 *      Author: root
 */
#include <unistd.h>
//#include <thread>
#include <sstream>
#include "stdlib.h"
#include "define.h"
#include "debug.h"
#include "AppConfig.h"
#include "AlmFilter.h"
#include "B_Interface.h"

extern GlobalDat gDat;
extern DeviceManager DevMgr;

extern const char * ids_mains_off_dev[3];
extern const char * ids_mains_off_filter[19];
extern const char * ids_spa_off_filter[8];
extern const char * ids_mains_high_filter[4];
extern const char * ids_mains_low_filter[3];
extern const char * ids_phase_off_filter[3];
extern const char * ids_mains_fail_filter[4];
const u32 v3pIdx_dev02[3] = {4,  5,  6};
const u32 v3pIdx_dev06[3] = {1,  2,  3};
const u32 v3pIdx_dev16[3] = {4,  5,  6};
const u32 DiIdx_dev02[5] = {16,  4,  3,  1, 2};
const u32 DiIdx_dev06[5] = {16, 17, 15, 14, 0};	// 0:invalid
const u32 DiIdx_dev16[5] = { 1,  4,  3,  2, 0};	// same as above
bool AlmFilter::init(vector<string> & devs) {
//	devIds.clear();
	ClearVector(devIds);
	for(auto & devId : devs) {
		if (devId.size() == 6)
			devId = string("0") + devId;
		if (!isIdAvailable(devId, 7)) {
			cout << "wrong device id in ACDistribution.conf.\n";
			return false;
		}
		devIds.push_back(devId);
	}
	return true;
}
void AlmFilter::updateDevs() {
	for(auto & devId : devIds) {
		const char * c = devId.c_str();
		char h,l;
		h = *c - '0';
		l = *(c + 1) - '0';
		char id = h * 10 + l;
		string sDevId = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + devId;
//		cout << __FUNCTION__ << ": devId = " << sDevId << endl;
		switch (id) {
		case 2:
			pDev_02 = DevMgr.findDevice(sDevId);
			break;
		case 6:
			pDev_06 = DevMgr.findDevice(sDevId);
			break;
		case 16:
			pDev_16 = DevMgr.findDevice(sDevId);
			break;
		}
	}
}
bool AlmFilter::findDevId(u32 devType, string & devId) {
	for(auto & it : devIds) {
//		cout << "AlmFilter::findDevId() - devIds.it" << it << endl;
		const char * shortDevId = it.c_str();
		u32 h = *shortDevId - '0';
		u32 l = *(shortDevId + 1) - '0';
		u32 ty = h * 10 + l;
		if (devType == ty) {
			devId = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + it;
			return true;
		}
	}
	return false;
}
void AlmFilter::filterAlarm(const char * id, bool b) {
	string Id(id);
//	cout << "before setFilter"<< Id << endl;
//	if (Id == "06025001")
//		cout << "now" << endl;
	u32 devType_1 = getSemaphoreDevType(Id);
	if (devType_1 >= DevMgr.maxDev)
		return;
	if (DevMgr.pDevs[devType_1] == NULL)
		return;
	string filterDevId;
	if (!findDevId(devType_1, filterDevId)) {
		printf("%s: can't find devId of sema id(%s) to filter!\n",
				__FUNCTION__,id);
		return;
	}
	Device * d = DevMgr.findDevice(filterDevId);
	if (d == NULL)
		return;
	if (!d->isReady())
		return;
	u32 semaIdx1_1 = getSemaphoreIdx(Id);
	DI * di = d->pDIs[semaIdx1_1 - 1];
	if (di != NULL) {
		di->setFilter(b);
	} else {
#if 0//DEBUG
		cout << "wrong filter id !" << endl;
#endif
	}
}
#if 0
void calcOnePhaseAlm(Device * pDev, u32 &semaIdx2_0,
		DI * pLost, DI * pLow, DI * pHigh, DI* pSuperHigh,
		float &refVal) {
	if ((pSuperHigh != NULL) && (refVal > pSuperHigh->threshold)) {
		pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	} else {
		if (pSuperHigh != NULL)
			pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		if (refVal > pHigh->threshold) {
			pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
			pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
			pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		} else if (refVal < pLost->threshold) {
			pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
			pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
			pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		} else if (refVal < pLow->threshold) {
			pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
			pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
			pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		} else {
			pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
			pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
			pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		}
	}
}
#endif
void AlmFilter::check_one_line(Device * pDev, float * v3p, const u32 * DiIdx,
		u32 semaIdx2_0) {
//	cout << "DiIdx[0] = " << DiIdx[0] << endl;
	DI * pNone = pDev->pDIs[DiIdx[0] - 1];			// none
	if (!pNone)
		return;
	DI * pLost = pDev->pDIs[DiIdx[1] - 1];			// some lost
	if (!pLost)
		return;
	DI * pLow = pDev->pDIs[DiIdx[2] - 1];			// some low
	if (!pLow)
		return;
	DI * pHigh = pDev->pDIs[DiIdx[3] - 1];			// some high
	if (!pHigh)
		return;
	DI * pSuperHigh = NULL;
	if (DiIdx[4] != 0) {
		pSuperHigh = pDev->pDIs[DiIdx[4] - 1];	// some high
		if (!pSuperHigh)
			return;
	}
	float refVal;
//	if (calcAllAlm3Phase(v3p, false, pNone->threshold, refVal)) {	// 停电
//		pNone->fsuCalcReverse(/*pDev->fullId, */semaIdx2_0, true, refVal);
//		pLost->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0, true, refVal);
//		pLow ->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0, true, refVal);
//		pHigh->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0, false, refVal);
//	} else {
		pNone->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0,
//				false,
				calcAllAlm3Phase(v3p, false, pNone->threshold, refVal),
				refVal);
		pLost->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0,
				calcAnyAlm3Phase(v3p, false, pLost->threshold, refVal), refVal);
		pLow ->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0,
				calcAnyAlm3Phase(v3p, false, pLow->threshold, refVal), refVal);
//		refVal = maxV(v3p);
		pHigh->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0,
				calcAnyAlm3Phase(v3p, true, pHigh->threshold, refVal), refVal);
		if (pSuperHigh != NULL) {	// super high alarm
			pSuperHigh->fsuCalcReverse(/*pDev->fullId,*/ semaIdx2_0,
				calcAnyAlm3Phase(v3p, true, pSuperHigh->threshold, refVal), refVal);
		}
//	}
#if 0
	if (calcAllAlm3Phase(v3p, false, noneThrd, refVal)) {
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	} else if (calcAnyAlm3Phase(v3p, false, noneThrd, refVal)) {
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	} else if (calcAnyAlm3Phase(v3p, false, lowThrd, refVal)) {
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	} else if (calcAnyAlm3Phase(v3p, true, highThrd, refVal)) {
		refVal = maxV(v3p);
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		if (pSuperHigh != NULL) {	// super high alarm
			if (calcAnyAlm3Phase(v3p, true, pSuperHigh->threshold, refVal))
				pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
			else
				pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		}
	} else {// no alarm
		refVal = minV(v3p);
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		if (pSuperHigh != NULL) 	// super high alarm
			pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	}
#endif
#if 0
	if (calcAllAlm3Phase(v3p, false, noneThrd, refVal)) {
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLost->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pLow ->fsuCalcReverse(pDev->fullId, semaIdx2_0, true, refVal);
		pHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		if (pSuperHigh != NULL) 	// super high alarm
			pSuperHigh->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
	} else {
		pNone->fsuCalcReverse(pDev->fullId, semaIdx2_0, false, refVal);
		calcOnePhaseAlm(pDev, semaIdx2_0, pLost, pLow, pHigh, pSuperHigh, *(v3p + 0));
		calcOnePhaseAlm(pDev, semaIdx2_0, pLost, pLow, pHigh, pSuperHigh, *(v3p + 1));
		calcOnePhaseAlm(pDev, semaIdx2_0, pLost, pLow, pHigh, pSuperHigh, *(v3p + 2));
	}
#endif
}
void AlmFilter::theDevPowerAlmCheck(Device * pDev, const u32 * v3pIdx, const u32 * DiIdx,
		bool bGrp) {
	if (!pDev)
		return;
	if (!pDev->isReady())
		return;
	AI * pVa = pDev->pAIs[v3pIdx[0] - 1];
	if (!pVa)
		return;
	AI * pVb = pDev->pAIs[v3pIdx[1] - 1];
	if (!pVb)
		return;
	AI * pVc = pDev->pAIs[v3pIdx[2] - 1];
	if (!pVc)
		return;
	float v3p[3];
	string v1, v2, v3;
	if (!bGrp) {
		if (pVa->getValStr(0, v1) && pVb->getValStr(0, v2) && pVc->getValStr(0, v3)) {
			v3p[0] = atof(v1.c_str());
			v3p[1] = atof(v2.c_str());
			v3p[2] = atof(v3.c_str());
			check_one_line(pDev, v3p, DiIdx, 0);
		}
	} else {
		vector<string> ssa;
		vector<string> ssb;
		vector<string> ssc;
		if (pVa->getGrpValStr(ssa) && pVb->getGrpValStr(ssb) && pVc->getGrpValStr(ssc)) {
			u32 sz = ssa.size();
			for (u32 i = 0; i < sz; ++i) {
				v3p[0] = atof(ssa[i].c_str());
				v3p[1] = atof(ssb[i].c_str());
				v3p[2] = atof(ssc[i].c_str());
				check_one_line(pDev, v3p, DiIdx, i);
			}
		}
	}
}
void AlmFilter::filterAlarms(const char * array[], u32 sz, bool b) {
	for (u32 i = 0; i < sz; ++i) {
//		cout << "filterAlarms" << array[i] << endl;
		filterAlarm(array[i], b);
	}
}
int AlmFilter::chkAlmStat(Device * p, u8 semaIdx1_1, bool bGrp, bool all) {
	if (p != NULL) {
		if (!p->isReady())
			return flag_unavail;
		if (!bGrp) {
			u8 out;
			if (!p->getDIStatById(semaIdx1_1, 0, out))
				return flag_unavail;
			else
				return (out > 0) ? flag_alm : flag_no_alm;
		} else {
			vector<u8> out;
			if (!p->getDIsStatById(semaIdx1_1, out))
				return flag_unavail;
			else {
				if (out.size() > 0) {
					if (all) {
							for(auto & it : out) {
								if (it == 0)
									return flag_no_alm; 	// 只要还有没告警的路，就不算
							}
							return flag_alm;	// 全部才算
					} else {
						for(auto & it : out) {
							if (it > 0)
								return flag_alm;
						}
					}
				} else
					return flag_unavail;
			}
		}
	}
	return flag_no_alm;
}
void AlmFilter::chkMainsOff() {
	_mainsOff[0] = chkAlmStat(pDev_16, DiIdx_dev16[0], true, true);
	_mainsOff[1] = chkAlmStat(pDev_02, DiIdx_dev02[0], true, true);
	_mainsOff[2] = chkAlmStat(pDev_06, DiIdx_dev06[0], true, true);
#if DEBUG_ALM_CHK_FILTER
		cout << " ## mainsOff state:"
				<< _mainsOff[0] << ","
				<< _mainsOff[1] << ","
				<< _mainsOff[2] << endl;
#endif
	int calc = _mainsOff[0] + _mainsOff[1] + _mainsOff[2];
	bSiteOff = (calc >= flag_double_alm);
}
bool AlmFilter::chkMainsLost(u32 & rptIdx) {
	_mainsLost[0] = chkAlmStat(pDev_16, DiIdx_dev16[1], true, false);
	_mainsLost[1] = chkAlmStat(pDev_02, DiIdx_dev02[1], true, false);
	_mainsLost[2] = chkAlmStat(pDev_06, DiIdx_dev06[1], true, false);
	for (u32 i = 0; i < 3; ++i) {
		if (_mainsLost[i] >= flag_alm) {
			mainsLostId_BInt = ids_phase_off_filter[i];
#if DEBUG_ALM_CHK_FILTER
		cout << " ## mainsLost state:"
				 << _mainsLost[0]
		  << "," << _mainsLost[1]
		  << "," << _mainsLost[2]  << endl;
#endif
			rptIdx = i;
			return true;
		}
	}
	return false;
}
bool AlmFilter::chkMainsLow(u32 & rptIdx) {
	_mainsLow[0] = chkAlmStat(pDev_16, DiIdx_dev16[2], true, false);
	_mainsLow[1] = chkAlmStat(pDev_02, DiIdx_dev02[2], true, false);
	_mainsLow[2] = chkAlmStat(pDev_06, DiIdx_dev06[2], true, false);
	for (u32 i = 0; i < 3; ++i) {
		if (_mainsLow[i] >= flag_alm) {
			mainsLowId_BInt = ids_mains_low_filter[i];
#if DEBUG_ALM_CHK_FILTER
		cout << " ## mainsLow state:"
		       << _mainsLow[0]
		<< "," << _mainsLow[1]
		<< "," << _mainsLow[2] << endl;
#endif
			rptIdx = i;
			return true;
		}
	}
	return false;
}
bool AlmFilter::chkMainsHigh(u32 & rptIdx) {
	_mainsHigh[0] = chkAlmStat(pDev_16, DiIdx_dev16[3], true, false);
	_mainsHigh[1] = chkAlmStat(pDev_02, DiIdx_dev02[4], true, false);
	_mainsHigh[2] = chkAlmStat(pDev_02, DiIdx_dev02[3], true, false);
	_mainsHigh[3] = chkAlmStat(pDev_06, DiIdx_dev06[3], true, false);
	for (u32 i = 0; i < 4; ++i) {
		if (_mainsHigh[i] >= flag_alm) {
			mainsHighId_BInt = ids_mains_high_filter[i];
			rptIdx = i;
#if DEBUG_ALM_CHK_FILTER
			cout << " ## mainsHigh state:"
					 << _mainsHigh[0]
			  << "," << _mainsHigh[1]
			  << "," << _mainsHigh[2]
			  << "," << _mainsHigh[3] << endl;
#endif
			return true;
		}
	}
	return false;
}

void AlmFilter::chkAndFilterAlarm() {
//	cout << __FUNCTION__ << "\t\t\t\t\t\t\t******\n";
	updateDevs();
	theDevPowerAlmCheck(pDev_16, v3pIdx_dev16, DiIdx_dev16, true);
	theDevPowerAlmCheck(pDev_02, v3pIdx_dev02, DiIdx_dev02, true);
	theDevPowerAlmCheck(pDev_06, v3pIdx_dev06, DiIdx_dev06, true);
	// order is 16 > 02 > 06
	chkMainsOff();
	filterAlarms(ids_mains_off_filter, 19, bSiteOff); // 条件过滤 mains_off_filter 表里的告警

	if (bSiteOff) { // confirm siteOff
		u32 rptIdx = -1;
		// 选择上报的告警 id
		if (_mainsOff[0] >= flag_alm) {
			mainsOffId_BInt = ids_mains_off_dev[0];
			rptIdx = 0;
		} else if (_mainsOff[1] >= flag_alm) {
			mainsOffId_BInt = ids_mains_off_dev[1];
			rptIdx = 1;
		}
		for (u32 i = 0; i < 3; ++i)
			filterAlarm(ids_mains_off_dev[i], rptIdx != i); // 只输出一条,过滤其它设备的告警
	} else { // 市电未中断
		for (u32 i = 0; i < 3; ++i)
			filterAlarm(ids_mains_off_dev[i], false); // 不过滤
		filterAlarms(ids_spa_off_filter, 8, (_mainsOff[2] >= flag_alm)); // 条件过滤 spa_off_filter 表里的告警

		if (_mainsOff[2] < flag_alm) { // 开关电源市电也没断
			filterAlarms(ids_spa_off_filter, 8, false);
			bool bFilterSPS = false;
			u32 rptIdx;
			if (chkMainsHigh(rptIdx)) {
				bFilterSPS = true;
				for (u32 i = 0; i < 4; ++i)
					filterAlarm(ids_mains_high_filter[i], rptIdx != i); // 只报一条，过滤其它
			} else {
				filterAlarms(ids_mains_high_filter, 4, false); // 不过滤
			}

			if (chkMainsLost(rptIdx)) {
				bFilterSPS = true;
				for (u32 i = 0; i < 3; ++i)
					filterAlarm(ids_phase_off_filter[i], rptIdx != i); // 只报一条，过滤其它
			} else {
				filterAlarms(ids_phase_off_filter, 3, false); // 不过滤
			}

			if (chkMainsLow(rptIdx)) {
				bFilterSPS = true;
				for (u32 i = 0; i < 3; ++i)
					filterAlarm(ids_mains_low_filter[i], rptIdx != i); // 只报一条，过滤其它
			} else
				filterAlarms(ids_mains_low_filter, 3, false); // 不过滤

			filterAlarms(ids_mains_fail_filter, 4, bFilterSPS); // 条件过滤相关告警
		} else {
			filterAlarms(ids_mains_fail_filter, 4, false); // 条件过滤相关告警
			filterAlarms(ids_spa_off_filter, 8, true);
		}
	}
}
bool AlmFilter::calcAnyAlm3Phase(float * v3p, bool big, float & thrd, float & ref) {
	bool rtn = false;
	for (int i = 0; i < 3; ++i) {
		if (big) {
			if (v3p[i] > thrd) {
				rtn = true;
			}
		} else {
			if (v3p[i] < thrd) {
				rtn = true;
			}
		}
	}
	ref = big ? maxV(v3p) : minV(v3p);
	return rtn;
}
bool AlmFilter::calcAnyAlmGrp(vector<string> vals, bool big, float & thrd, float & ref) {
	bool rtn = false;
	u32 sz = vals.size();
	float max = 0;
	float min = 100000;
	for (u32 i = 0; i < sz; ++i) {
		float v = atof(vals[i].c_str());
		if (big) {
			if (v > thrd)
				rtn = true;
			if (v > max)
				max = v;
		} else {
			if (v < thrd)
				rtn = true;
			if (v < min)
				min = v;
		}
	}
	ref = big ? max : min;
	return rtn;
}
bool AlmFilter::calcAllAlm3Phase(float * v3p, bool big, float & thrd, float & ref) {
	int cnt = 0;
	for (int i = 0; i < 3; ++i) {
		if (big) {
			if (v3p[i] > thrd) {
				cnt++;
			}
		} else {
			if (v3p[i] < thrd)
				cnt++;
		}
	}
	ref = big ? maxV(v3p) : minV(v3p);
	return (cnt >= 3) ;
}
bool AlmFilter::calcAllAlmGrp(vector<string> vals, bool big, float & thrd, float & ref) {
	bool rtn = false;
	u32 sz = vals.size();
	float max = 0;
	float min = 100000;
	for (u32 i = 0; i < sz; ++i) {
		float v = atof(vals[i].c_str());
		if (big) {
			if (v <= thrd) {
				rtn = true;
			}
			if (v > max)
				max = v;
		} else {
			if (v >= thrd)
				rtn = true;
			if (v < min)
				min = v;
		}
	}
	ref = big ? max : min;
	return rtn ;

}
float AlmFilter::maxV(float * v3p) {
	float a = v3p[0];
	float b = v3p[1];
	float c = v3p[2];
	return (a>b?(a>c?a:c):(b>c?b:c));
}
float AlmFilter::minV(float * v3p) {
	float a = v3p[0];
	float b = v3p[1];
	float c = v3p[2];
	return (a<b?(a<c?a:c):(b<c?b:c));
}
