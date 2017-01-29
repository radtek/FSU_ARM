/*
 * AlmFilter.h
 *
 *  Created on: 2016年10月16日
 *      Author: root
 */

#ifndef INC_ALMFILTER_H_
#define INC_ALMFILTER_H_

#include "Device.h"

const int flag_unavail = 0;
const int flag_no_alm = 1;
const int flag_alm = 3;
const int flag_double_alm = 6;
class Device;
class AlmFilter {
public:
	bool init(vector<string> & devs);
	void chkAndFilterAlarm();
private:
	void filterAlarm(const char * id, bool b);
	void filterAlarms(const char * array[], u32 sz, bool b);
	void theDevPowerAlmCheck(Device * pDev, const u32 * v3pIdx, const u32 * DiIdx,
			bool bGrp);
	int chkAlmStat(Device * p, u8 diIdx, bool bGrp, bool all);
	void chkMainsOff();
	bool chkMainsLost(u32 & rptIdx);
	bool chkMainsLow(u32 & rptIdx);
	bool chkMainsHigh(u32 & rptIdx);
	void check_one_line(Device * pDev, float * v3p, const u32 * DiIdx, u32 semaIdx2_0);
	bool calcAnyAlm3Phase(float * v3p, bool big, float & thrd, float & ref);
	bool calcAnyAlmGrp(vector<string> vals, bool big, float & thrd, float & ref);
	bool calcAllAlm3Phase(float * v3p, bool big, float & thrd, float & ref);
	bool calcAllAlmGrp(vector<string> vals, bool big, float & thrd, float & ref);
	float maxV(float * v3p);
	float minV(float * v3p);
	bool findDevId(u32 devType, string & devId);
	void updateDevs();
private:
	Device * pDev_02 = NULL;	// 低压配电
	Device * pDev_06 = NULL;	// 开关电源
	Device * pDev_16 = NULL;	// 智能电表
	vector<string> devIds;
	int _mainsOff[3];
	int _mainsHigh[4];
	int _mainsLost[3];
	int _mainsLow[3];
	int bSiteOff;
//	bool bFilterSPS；// 过滤基站市电异常?
	// ---- B接口显示串 ----
	string mainsOffId_BInt;
	string mainsHighId_BInt;
	string mainsLowId_BInt;
	string mainsLostId_BInt;
};


#endif /* INC_ALMFILTER_H_ */
