/*
 * AppConfig.cpp
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */

#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include "xmlHdl.h"
#include <libxml/tree.h>
#include "B_Interface.h"
#include "AppConfig.h"
#include "DevComm.h"

using namespace std;
using namespace BInt;

GlobalDat gDat;
int NetMode = 0;
int fsuMode = 2;
const char *cfgPaths[GIDX_MAX_SIZE] = {
		"/config/SC_Login/UserName",	// 0
		"/config/SC_Login/PaSCword", 	// 1
		"/config/FSU/FsuId", 			// 2
		"/config/FSU/FsuCode",			// 3
		"/config/FSU/IP", 				// 4
		"/config/FSU/ComType",			// 5
		"/config/FSU/FSUMode", 			// 6
		"/config/FTP/UserName", 		// 7
		"/config/FTP/Password",			// 8
		"/config/IPSec/IPSecUser",		// 9
		"/config/IPSec/IPSecPWD",		//10
		"/config/IPSec/IPSecIP", 		//11
		"/config/SC/SCIP_Login", 		//12
		"/config/SC/SCIP_Getter", 		//13
		"/config/SC/SCPort", 			//14
		"/config/SC/SCMode",			//15
		"/config/Site/Code", 			//16
		"/config/Site/Type",			//17
		"/config/Misc/BaudRate485"		//18
};
//extern  pthread_mutex_t fsuInfo_lock;
extern std::mutex mtx_fsuInfo;
//pthread_mutex_t Reboot_lock = PTHREAD_MUTEX_INITIALIZER;
std::mutex mtx_reboot;
void getOccRate(string & c, string &m) {
//	pthread_mutex_lock(&fsuInfo_lock);
	mtx_fsuInfo.lock();
	c = gDat.cpuOccRate;
	m = gDat.memOccRate;
//	pthread_mutex_unlock(&fsuInfo_lock);
	mtx_fsuInfo.unlock();
}
u32 getRebootFlag() {
	u32 rtn;
//	pthread_mutex_lock(&Reboot_lock);
	mtx_reboot.lock();
	rtn = gDat.rebootFlag;
//	pthread_mutex_unlock(&Reboot_lock);
	mtx_reboot.unlock();
	return rtn;
}
void setRebootFlag() {
//	pthread_mutex_lock(&Reboot_lock);
	RLock(mtx_reboot);
	gDat.rebootFlag = 1;
//	pthread_mutex_unlock(&Reboot_lock);
}
char * get_exe_path(char * buf, int count) {
	int i;
	int rslt = readlink("/proc/self/exe", buf, count - 1);
	if (rslt < 0 || (rslt >= count - 1))
		return NULL;
	buf[rslt] = '\0';
	for (i = rslt; i >= 0; i--) {
		if (buf[i] == '/') {
			buf[i + 1] = '\0';
			break;
		}
	}
	return buf;
}
bool getConfigVal(xmlDoc * pDoc, const xmlChar *xpath, string & value) {
	xmlXPathObjectPtr rst = getNodeset(pDoc, xpath);
	bool rtn = false;
	if (rst) {
		xmlNodeSetPtr nodeset = rst->nodesetval;
		xmlNodePtr cur;
		if (nodeset->nodeNr > 0) {
			cur = nodeset->nodeTab[0];
//			xmlXPathFreeObject(rst);
			if (cur) {
				xmlChar * content = xmlNodeGetContent(cur);
				value = string((const char *)content);
				xmlFree(content);
				rtn = true;
			}
		}
		xmlXPathFreeObject(rst);
	}

	return rtn;
}
void getDeviceList(xmlDoc * pDoc, vector<BInt::devIdCode> & vDev) {
	const xmlChar * xpath = (const xmlChar *) ("//Device");
	xmlXPathObjectPtr rst = getNodeset(pDoc, xpath);
	if (rst) {
		xmlNodeSetPtr nodeset = rst->nodesetval;
		xmlNodePtr cur;

		for (int i = 0; i < nodeset->nodeNr; i++) {
			cur = nodeset->nodeTab[i];
			if (cur->type != XML_ELEMENT_NODE)
				continue;
			xmlAttr * attr = cur->properties;
			const xmlChar * id;
			const xmlChar * code;
			if (attr) {
				if (!xmlStrcmp(attr->name, (const xmlChar *) "Id")) {
					id = attr->children->content;
					attr = attr->next;
					if (attr) {
						if (!xmlStrcmp(attr->name, (const xmlChar *) "Code"))
							code = attr->children->content;
					}
				}
			}
			if (BInt::isIdAvailable(string((char *) id), 7)) {
				BInt::devIdCode idCode;
				idCode.Id = (char *) id;
				idCode.Code = (char *) code;
				vDev.push_back(idCode);
			} else {
				cout << __FUNCTION__ << ": wrong device id.\n";
			}
		}
		xmlXPathFreeObject(rst);
	}
}
bool setConfig(u32 idx, string s, bool isInit) {
	if (idx >= GIDX_MAX_SIZE)
		return false;

	if (s.size() <= gArrayIdxLen[idx])
		gDat.cfgs[idx] = s;
	else
		gDat.cfgs[idx] = s.substr(0, gArrayIdxLen[idx]);

	if (!isInit) {
		string filename = gDat.exePath + string("cfg/FSUapp.conf");
		xmlDoc *pDoc = xmlReadFile(filename.c_str(), NULL, 0);
		if (pDoc == NULL) {
			printf("error: could not find file %s\n", filename.c_str());
			return false;
		} else {
			bool ok = false;
			xmlChar *xpath = (xmlChar *) cfgPaths[idx];
			xmlXPathObjectPtr rst = getNodeset(pDoc, xpath);
			if (rst) {
				xmlNodeSetPtr nodeset = rst->nodesetval;
				xmlNodePtr cur;
				if (nodeset->nodeNr > 0) {
					cur = nodeset->nodeTab[0];
//					xmlXPathFreeObject(rst);
					if (cur) {
						xmlNodeSetContent(cur, BAD_CAST s.c_str());
						ok = true;
					}
				}
				xmlXPathFreeObject(rst);
			}
			if (ok) {
				xmlSaveFileEnc(filename.c_str(), pDoc, "UTF-8");
				/*free the document */
			} else {
				xmlFreeDoc(pDoc);
				return false;
			}
			xmlFreeDoc(pDoc);
		}
	}

	return true;
}
bool initConfig() {
	// read config
	gDat.rebootFlag = 0;
	char path[256];
	gDat.exePath = get_exe_path(path, 256);
	string p = gDat.exePath + string("cfg/FSUapp.conf");
	xmlDoc *pDoc = xmlReadFile(p.c_str(), NULL, 0);
	if (pDoc == NULL) {
		printf("error: could not find file %s\n", p.c_str());
		return false;
	} else {
		string content;
		for (u32 idx = 0; idx < GIDX_MAX_SIZE; ++idx) {
			xmlChar *xpath = (xmlChar *) cfgPaths[idx];
			if (getConfigVal(pDoc, xpath, content)) {
//#if DEBUG_INIT_DAT
//				printf("content: %s", xpath);
//#endif
				setConfig(idx, content, true);
#if DEBUG
				cout << "\t";
				string path = string((const char *)xpath);
				u32 sp = 40 - path.size();
				cout << xpath;
				for(u32 i = 0; i < sp; ++i)
					printf(" ");
				printf(" : %s\n", gDat.cfgs[idx].c_str());
#endif
			} else {
				printf("%s read error.\n", xpath);
				goto FALSE;
			}
		}
		gDat.port = 8080;//atoi(gDat.cfgs[GIDX_SCPORT].c_str());
		getDeviceList(pDoc, gDat.vecDev);
#if 0
		if (gDat.vecDev.size() > 0) {
			string devId0 = gDat.vecDev[0].Id;
			const char * s = devId0.c_str();
			u32 site = (s[6] - '0');
			if ((site <= 0) || (site > 4)) {
				cout << "Bad site num :" << site << endl;
				goto FALSE;
			}
			char t[2] = { 0, 0 };
			t[0] = s[6];
			gDat.siteType = string(t);
			gDat.thresholdSavePosOfDiFile = 7 + (t[0] - '0');

#if DEBUG
			cout << "site type is " << gDat.siteType << endl;
#endif
			char pre[3] = { '0', 0, 0 };
			pre[1] = s[6];
			gDat.semaIdPreString = string(pre);
#if DEBUG
			cout << "semaphore id pre string is " << gDat.semaIdPreString
					<< endl;
#endif
		} else
			goto FALSE;
#endif
		NetMode = atoi(gDat.cfgs[GIdX_SC_MODE].c_str());
		fsuMode = atoi(gDat.cfgs[GIdX_FSU_MODE].c_str());
		gDat.semaIdPreString = string("0") + gDat.cfgs[GIDX_SITE_TYPE];
		char t[2] = { 0, 0 };
		t[0] = *(gDat.semaIdPreString.c_str() + 1);
		gDat.siteType = string(t);
//		gDat.thresholdSavePosOfDiFile = 7 + (t[0] - '0');

		gDat.baudRate485 = atoi(gDat.cfgs[GIDX_BAUDRATE_485].c_str());
		gDat.almLevelStr[0] = "1";	//"一级";
		gDat.almLevelStr[1] = "2";	//"二级";
		gDat.almLevelStr[2] = "3";	//"三级";
		gDat.almLevelStr[3] = "4";	//"四级";

		gDat.almFlag[0] = "BEGIN";	//"结束";
		gDat.almFlag[1] = "END";	//"开始";
		gDat.invalidStr = "6";
		gDat.validStr = "0";
		gDat.datType_DI = "2";
		gDat.datType_AI = "3";
		gDat.datType_DO = "4";
		gDat.datType_AO = "5";
		gDat.cfgs[GIDX_FSUID] = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType
							+ DEV_TYPE_FSU + gDat.cfgs[GIDX_FSUID];
		gDat.cfgs[GIDX_FSUCODE] = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType
							+ DEV_TYPE_FSU + gDat.cfgs[GIDX_FSUCODE];
#if DEBUG
			cout << "FSU ID=" << gDat.cfgs[GIDX_FSUID]
				 << ", FSU Code=" << gDat.cfgs[GIDX_FSUCODE] << endl;
#endif

		for (auto & dev : gDat.vecDev) {
			dev.Id = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + dev.Id;
			dev.Code = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + dev.Code;
#if DEBUG
			cout << "Device ID = " << dev.Id << ", Code = " << dev.Code << endl;
#endif
//			switch (getDeviceType(dev.Id)) {
//			case DEV_TYPE_06_KaiGuanDianYuan:
//				gDat.devId_of_type06 = dev.Id;
//				break;
//			case DEV_TYPE_07_BattMeas:
//				gDat.devId_of_type07 = dev.Id;
//				break;
//			case DEV_TYPE_02_JiaoLiuPeiDianXiang:
//				gDat.devId_of_type02 = dev.Id;
//				break;
//			case DEV_TYPE_16_ZhiNengDianBiao:
//				gDat.devId_of_type16 = dev.Id;
//				break;
//			case DEV_TYPE_17_DoorGuard:
//				gDat.devId_of_type17 = dev.Id;
//				break;
//			case DEV_TYPE_18_JiFangHuanJing:
//				gDat.devId_of_type18 = dev.Id;
//				break;
//			case DEV_TYPE_19_TongXinZhongDuan:
//				gDat.devId_of_type19 = dev.Id;
//				break;
//			}
		}
		gDat.SCLoginIP = string("http://") + gDat.cfgs[GIDX_SCIP_LOGIN] + string(":") + gDat.cfgs[GIDX_SCPORT] + "/services/SCService";
		gDat.szSCLoginIP = gDat.SCLoginIP.c_str();
		gDat.SCGetterIP = string("http://") + gDat.cfgs[GIDX_SCIP_GETTER] + string(":") + gDat.cfgs[GIDX_SCPORT] + "/services/SCService";
		gDat.szSCGetterIP = gDat.SCGetterIP.c_str();
	}

	FALSE: xmlFreeDoc(pDoc);

	return true;
}
bool load_init_devCommErrcfg() {
	gDat.vecDevCommErrCfg.clear();
	vector<vector<string> > dat;
	string p = gDat.exePath + string("cfg/dev19.csv");
	if (!readCsv(dat, p))
		return false;
	u32 ins = atoi(dat[1][1].c_str());
	if (ins > 255){
		cout << endl << "Gatherer's install configure error !!" << endl << endl;
		return false;
	}
	gDat.gatherersInstallDat8 = ins;
#if DEBUG_INIT_DAT
	cout << " ------------- Device Comm error indx -------------" << endl;
#endif
	for (u32 x = 1; x < dat.size(); x++) {
		if (skipTheLine(dat[x]))
			continue;
		if (checkQuit(dat[x], x, p, 3))
			return false;

		if (x != 1) {
			if (!isIdAvailable(dat[x][0], 8))
				cout << "wrong id of devCommErr's semaphore !" << endl;
			else
				gDat.vecDevCommErrCfg.push_back(make_pair(dat[x][0], dat[x][1]));
		}
#if DEBUG_INIT_DAT
		cout << "itm[" << x - 1 << "] - (" << dat[x][0]
			 << ", " << dat[x][1] << ", " << dat[x][2] << ")" << endl;
#endif
	}
	return true;
}

bool initDev18Dat () {
	gDat.dev18CfgTbl.clear();
	vector<vector<string> > dat;
	string p = gDat.exePath + string("cfg/dev18.csv");
	if (!readCsv(dat, p))
		return false;
	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> &col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 3))
			return false;
		dev18CfgItm itm;
		itm.addr = atoi(col[0].c_str());
		if ((itm.addr < 1) || (itm.addr > 8)) {
			cout << "wrong addr of ext board !" << endl;
			return false;
		}

		if (!isIdAvailable(col[1], 7)) {
			cout << "wrong id of device !" << endl;
			return false;
		}
		itm.sc_devId = col[1];
		if (!isIdAvailable(col[2], 8)) {
			cout << "wrong id of semaphore !" << endl;
			return false;
		}
		itm.semaId = col[2];
		gDat.dev18CfgTbl.push_back(itm);
#if DEBUG_INIT_DAT
		cout << "itm[" << x - 1 << "] - (" << dat[x][0]
			 << ", " << dat[x][1] << ", " << dat[x][2] << ")" << endl;
#endif
		string fullDevId = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +  itm.sc_devId;
		bool find = false;
		for (const auto & dev : gDat.vecDev) {
			if (dev.Id == fullDevId) {
				find = true;
				break;
			}
		}
		if (!find) {
			devIdCode idcd;
			idcd.Id = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +  itm.sc_devId;
			idcd.Code = idcd.Id;
			gDat.vecDev.push_back(idcd);
		}
	}
#if DEBUG_INIT_DAT
	for (const auto & dev : gDat.vecDev) {
		cout << "vecDev itm:" << dev.Id << endl;
	}
#endif

	return true;
}
bool isGrpId(const string &devIdOfSema, const string &devIdInGrpTbl) {
	if (devIdOfSema == devIdInGrpTbl)
		return true;
	else {
		bool find = false;
		string modelIdx;
		for (const auto i : gDat.productDefCfgInitTbl) {
			if (devIdOfSema == i.productModelIdx) {
				find = true;
				break;
			}
		}
		if (find)
			return true;
	}
	return false;
}
bool load_init_DI() {
#if 0
	vector<vector<string> > data;
	string p = gDat.exePath + string("cfg/init_DI.csv");
	if (!readCsv(data, p))
		return false;
	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 21))
			return false;

		string dId_mod = data[x][0];
		size_t sz = dId_mod.size();
		if (sz >= 6) {
			if (sz == 6)
				dId_mod = string("0") + dId_mod;
			if(!isIdAvailable(dId_mod, 7)) {
				outputInitItm(data[x]);
				cout << "wrong device id or model idx of DI !" << endl;
				return false;
			}
		}
		sz = data[x][1].size();
		if ((sz != 7) && (sz != 8)) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's id !" << endl;
			return false;
		}
		if (data[x][1].size() == 7)
			data[x][1] = string("0") + data[x][1];
		string id = data[x][1];
		const char * s = id.c_str();
		if ((s[0] == 0) && (s[1] == 0)) {
			outputInitItm(data[x]);
			cout << "wrong device type of DI's id !" << endl;
			return false;
		}

		string aId = data[x][2];
		sz = aId.size();
		if (sz != 0) {
			if ((sz != 7) && (sz != 8)) {
				outputInitItm(data[x]);
				cout << "wrong format of DI's aId !" << endl;
				return false;
			}
			if (aId.size() == 7)
				aId = string("0") + aId;
			if (!isIdAvailable(aId, 8)) {
				outputInitItm(data[x]);
				cout << "wrong format of DI's aId ( !" << aId << ")" << endl;
				return false;
			}
//			cout << "id=" << id << "," << "aid=" << aId << endl;
		}

		string logic;
		if (data[x][3].size() != 0) {
			logic = data[x][3];
			s = logic.c_str();
			if ((*s == 'h') || (*s == 'H'))
				logic = string("h");
			else if ((*s == 'l') || (*s == 'L'))
				logic = string("l");
			else {
				outputInitItm(data[x]);
				cout << "wrong format of DI's logic !" << endl;
				return false;
			}
		}

		string backlash = data[x][4];
		if (atof(backlash.c_str()) < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of AI's backlash !" << endl;
			return false;
		}

		string delayA = data[x][5];
		int dly = atoi(delayA.c_str());
		if (dly < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's delay !" << endl;
			return false;
		}

		string delayB = data[x][6];
		dly = atoi(delayB.c_str());
		if (dly < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's delay !" << endl;
			return false;
		}

		string delayC = data[x][7];
		dly = atoi(delayC.c_str());
		if (dly < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's delay !" << endl;
			return false;
		}

		string delayD = data[x][8];
		dly = atoi(delayD.c_str());
		if (dly < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's delay !" << endl;
			return false;
		}

		string thrA = data[x][9];
		if (atof(thrA.c_str()) < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's threshold-A !" << endl;
			return false;
		}

		string thrB = data[x][10];
		if (atof(thrB.c_str()) < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's threshold-B !" << endl;
			return false;
		}

		string thrC = data[x][11];
		if (atof(thrC.c_str()) < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's threshold-C !" << endl;
			return false;
		}

		string thrD = data[x][12];
		if (atof(thrD.c_str()) < 0) {
			outputInitItm(data[x]);
			cout << "wrong format of DI's threshold-D !" << endl;
			return false;
		}

		string levelA = data[x][13];
		if (levelA.size() == 0) {
			levelA = string();
		} else {
			int lvlA = atoi(levelA.c_str());
			if ((lvlA >= 1) && (lvlA <= 4)) {
				char c[2];
				c[0] = '0' + lvlA;
				c[1] = 0;
				levelA = string(c);
			} else {
				outputInitItm(data[x]);
				cout << "wrong format of DI's level-A !" << endl;
				return false;
			}
		}

		string levelB = data[x][14];
		if (levelB.size() == 0) {
			levelB = string();
		} else {
			int lvlB = atoi(levelB.c_str());
			if ((lvlB >= 1) && (lvlB <= 4)) {
				char c[2];
				c[0] = '0' + lvlB;
				c[1] = 0;
				levelB = string(c);
			} else {
				outputInitItm(data[x]);
				cout << "wrong format of DI's level-B !" << endl;
				return false;
			}
		}

		string levelC = data[x][15];
		if (levelC.size() == 0) {
			levelC = string();
		} else {
			int lvlC = atoi(levelC.c_str());
			if ((lvlC >= 1) && (lvlC <= 4)) {
				char c[2];
				c[0] = '0' + lvlC;
				c[1] = 0;
				levelC = string(c);
			} else {
				outputInitItm(data[x]);
				cout << "wrong format of DI's level-C !" << endl;
				return false;
			}
		}

		string levelD = data[x][16];
		if (levelD.size() == 0) {
			levelD = string();
		} else {
			int lvlD = atoi(levelD.c_str());
			if ((lvlD >= 1) && (lvlD <= 4)) {
				char c[2];
				c[0] = '0' + lvlD;
				c[1] = 0;
				levelD = string(c);
			} else {
				outputInitItm(data[x]);
				cout << "wrong format of DI's level-D !" << endl;
				return false;
			}
		}
		string level;
		s = gDat.siteType.c_str();
		switch (*s) {
		case '1':
			level = levelA;
			break;
		case '2':
			level = levelB;
			break;
		case '3':
			level = levelC;
			break;
		case '4':
			level = levelD;
			break;
		default:
			cout << "siteType error. \n";
			return false;
		}
		if (level.size() != 0) {	// 是告警，否则是状态
			bool bAIDef = aId.size() > 0;
			bool bCompDef = logic.size() > 0;

			if ((bAIDef && !bCompDef) || (!bAIDef && bCompDef)) {
				outputInitItm(data[x]);
				cout << "wrong configure of DI, miss sth." << endl;
				return false;
			}
		}
		stInitDI2 other = stInitDI2(delayA, delayB, delayC, delayD, thrA, thrB,
				thrC, thrD, levelA, levelB, levelC, levelD);
		bool bFindGrp = false;
//		cout << "DI's id(" << id << endl;
		for (auto & g : gDat.vecInitGRP) {
//			cout << "init grp itm, semaId = " << g.semaId << endl;
			if (id == g.semaId) {
				if (isGrpId(dId_mod, g.devId)) {
					bFindGrp = true;
					gDat.vecInitDI.push_back(
							stInitDI(dId_mod, id, aId, logic, backlash, other, data[x][17],
									data[x][18], data[x][19], &g, data[x][20]));
					break;
				}
			}
		}
		if (!bFindGrp)
			gDat.vecInitDI.push_back(
					stInitDI(dId_mod, id, aId, logic, backlash, other, data[x][17],
							data[x][18], data[x][19], NULL, data[x][20]));
	}
//	for (auto & it : gDat.vecInitDI) {
//		cout << it.id << endl;
//	}
#endif
	return true;
}
bool load_init_AI() {
#if 0
	vector<vector<string> > data;
	data.clear();
	string p = gDat.exePath + string("cfg/init_AI.csv");
	if (!readCsv(data, p))
		return false;
	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 5))
			return false;

		string dId_mod = data[x][0];
		size_t sz = dId_mod.size();
		if (sz >= 6) {
			if (sz == 6)
				dId_mod = string("0") + dId_mod;
			if(!isIdAvailable(dId_mod, 7)) {
				outputInitItm(data[x]);
				cout << "wrong device id or model idx of DI !" << endl;
				return false;
			}
		}
		sz = data[x][1].size();
		if ((sz != 7) && (sz != 8)) {
			outputInitItm(data[x]);
			cout << "wrong format of AI's id !" << endl;
			return false;
		}
		if (data[x][1].size() == 7)
			data[x][1] = string("0") + data[x][1];
		string id;
		if (!isIdAvailable(data[x][1], 8)) {
			outputInitItm(data[x]);
			cout << "wrong format of AI's id !" << endl;
			return false;
		}
		id = data[x][1];
		const char * s = id.c_str();
		if ((s[0] == 0) && (s[1] == 0)) {
			outputInitItm(data[x]);
			cout << "wrong device type of AI semaphore's id !" << endl;
			return false;
		}

		string abs = data[x][2];
		if (abs.size() != 0) {
			if (atof(abs.c_str()) <= 0) {
				outputInitItm(data[x]);
				cout << "wrong format of AI's Abs threshold !" << endl;
				return false;
			}
		}
		string rel = data[x][3];
		if (rel.size() != 0) {
			if (atof(rel.c_str()) <= 0) {
				outputInitItm(data[x]);
				cout << "wrong format of AI's Rel threshold !" << endl;
				return false;
			}
		}
		if ((abs.size() != 0) && (rel.size() != 0)) {
			outputInitItm(data[x]);
			cout << "Abs threshold and Rel threshold set both !" << endl;
			return false;
		}

		bool bFindGrp = false;
		for (auto & g : gDat.vecInitGRP) {
			if (id == g.semaId) {
				if (isGrpId(dId_mod, g.devId)) {
					bFindGrp = true;
					gDat.vecInitAI.push_back(
							stInitAI(dId_mod, id, abs, rel, data[x][4], data[x][5], &g));
					break;
				}
			}
		}
		if (!bFindGrp)
			gDat.vecInitAI.push_back(
					stInitAI(dId_mod, id, abs, rel, data[x][4], data[x][5], NULL));
	}
#endif
	return true;
}
bool load_init_GRP() {
#if 0
	vector<vector<string> > data;
	data.clear();
	string p = gDat.exePath + string("cfg/init_GRP.csv");
	if (!readCsv(data, p))
		return false;

	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 4))
			return false;

		size_t sz = data[x][1].size();
		if ((sz != 7) && (sz != 8)) {
			outputInitItm(data[x]);
			cout << "wrong format of semaphore's id !" << endl;
			return false;
		}
		if (data[x][1].size() == 7)
			data[x][1] = string("0") + data[x][1];
		string id = data[x][1];
		const char * s = id.c_str();
		if ((s[0] == 0) && (s[1] == 0)) {
			outputInitItm(data[x]);
			cout << "wrong device type of semaphore's group id !" << endl;
			return false;
		}

		if (data[x][2].size() == 7)
			data[x][2] = string("0") + data[x][2];	// 格式化为8位
		string gId = data[x][2];

		s = gId.c_str();
		sz = gId.size();
		if (!BInt::isIdAvailable(gId, 8)) {	// 非标准id, 判断有效性
			if (sz == 1) {
				if ((s[0] >= '0') && (s[0] <= '9')) {
					gId = string("0") + gId;
				} else {
					outputInitItm(data[x]);
					cout << "unknown group Id format !" << endl;
					return false;
				}
			} else if (sz == 2) {
				if (!BInt::isIdAvailable(gId, 2)) {
					outputInitItm(data[x]);
					cout << "wrong format of semaphore's group Id !" << endl;
					return false;
				}
			} else if (sz == 3) {
				if (!((s[0] == 'n') && ((s[1] >= '0') && (s[1] <= '9'))
						&& ((s[2] >= '0') && (s[2] <= '9')))) {
					outputInitItm(data[x]);
					cout << "wrong format of semaphore's group Id !" << endl;
					return false;
				}
			} else {
				outputInitItm(data[x]);
				cout << "unknown group Id format !" << endl;
				return false;
			}
		}
#if DEBUG_INIT_DAT
		cout << "group def: " << data[x][0] << "," << id << "," << gId << endl;
#endif
		gDat.vecInitGRP.push_back(stInitGRP(data[x][0], id, gId));
	}
#endif
	return true;
}
bool load_init_DOAO() {
#if 0
	for (const auto & it : gDat.vecInitlist) {

		const char * s = it.semaId.c_str();
		if ((s[2] == '2') || (s[2] == '3')) {
			bool bFindGrp = false;
			stInitGRP ig("","","");
			for (auto & g : gDat.vecInitGRP) {
				if (it.semaId == g.semaId) {
					if (isGrpId(it.devId, g.devId)) {
						bFindGrp = true;
						ig = g;
						break;
					}
				}
			}
			BInt::stInitGRP * p = bFindGrp ? &ig : NULL;
			if (s[2] == '2') {
#if DEBUG_INIT_DAT
				cout << "init dat(DO):" << it.devId << "," << it.semaId << "," << it.name << endl;
#endif
				gDat.vecInitDO.push_back(stInitDO(it.devId,it.semaId, it.name, p));
			} else if (s[2] == '3') {
#if DEBUG_INIT_DAT
				cout << "init dat(AO):" << it.devId << "," << it.semaId << "," << it.name << endl;
#endif
				gDat.vecInitAO.push_back(stInitAO(it.devId,it.semaId, it.name, p));
			}
		}
	}
#endif
	return true;
}
//bool load_init_list() {
//#if 0
//	string p = gDat.exePath + string("cfg/init_list.csv");
//	vector<vector<string> > data;
//	if (!readCsv(data, p))
//		return false;
//	for (u32 x = 1; x < data.size(); x++) {
//		if (skipTheLine(data[x]))
//			continue;
//		if (checkQuit(data[x], x, p, 4))
//			return false;
//
//		string devId = data[x][0];	// dev id
//		string semaId = data[x][1];	// sema id
//		string thrd = data[x][2];	// threshold
//		string name = data[x][3];
//		unsigned int sz = semaId.size();
//// XXX
////		if (!BInt::isIdAvailable(semaId, sz)) {
////			if (string("IPSECPWD") == semaId)
////				gDat.cfgs[GIDX_IPSECUSER] = thrd;
////			else if (string("IPSECUSER") == semaId)
////				gDat.cfgs[GIDX_IPSECPWD] = thrd;
////			else if (string("FTPUSER") == semaId)
////				gDat.cfgs[GIDX_FTP_USERNAME] = thrd;
////			else if (string("FTPPWD") == semaId)
////				gDat.cfgs[GIDX_FTP_PASSWORD] = thrd;
////			else if (string("IPSecIP") == semaId)
////				gDat.cfgs[GIDX_IPSECIP] = thrd;
////			else if (string("SCIP_Login") == semaId)
////				gDat.cfgs[GIDX_SCIP_LOGIN] = thrd;
////			else if (string("SCIP_Getter") == semaId)
////				gDat.cfgs[GIDX_SCIP_GETTER] = thrd;
////			else {
////				outputInitItm(data[x]);
////				cout << "unknown id !" << endl;
////			}
////		} else {
//			if ((sz != 7) && (sz != 8)) {
//				outputInitItm(data[x]);
//				cout << "wrong format of semaphore's id !" << endl;
//				continue;
//			}
//			if (semaId.size() == 7)
//				semaId = string("0") + semaId;
//			gDat.vecInitlist.push_back(stInit4ADIO(devId, semaId, thrd, name));
////		}
//	}
//#endif
//	return true;
//}
bool init_semaphores() {
#if 0
//	int siteType = -1;
	const char * s = gDat.siteType.c_str();
	if(!load_init_list())
		return false;
	if(!load_init_GRP()) // must before load_init_DI() and load_init_AI()
		return false;
	if(!load_init_DI())
		return false;
	if(!load_init_AI())
		return false;
	if(!load_init_DOAO())
		return false;

	for (auto & di : gDat.vecInitDI) {
		switch (*s) {
		case '1':
			di.threshold_sel = di.other.thresholdA;
			di.level_sel = di.other.levelA;
			di.delay_sel = di.other.delayA;
			break;
		case '2':
			di.threshold_sel = di.other.thresholdB;
			di.level_sel = di.other.levelB;
			di.delay_sel = di.other.delayB;
			break;
		case '3':
			di.threshold_sel = di.other.thresholdC;
			di.level_sel = di.other.levelC;
			di.delay_sel = di.other.delayC;
			break;
		case '4':
			di.threshold_sel = di.other.thresholdD;
			di.level_sel = di.other.levelD;
			di.delay_sel = di.other.delayD;
			break;
		}
		for (const auto & grp : gDat.vecInitGRP) {		// group id
			if (di.id == grp.semaId) {
				di.gId = grp.grpId;
				break;
			}
		}
		for (const auto & thr : gDat.vecInitlist) {		// threshold cover
			if (di.id == thr.semaId) {
				di.name = thr.name;
				if (!thr.setVal.empty())
					di.threshold_sel = thr.setVal;
				break;
			}
		}
		for (const auto & grp : gDat.vecInitGRP) {		// group data
			if (di.id == grp.semaId) {
				di.bGrp = grp.bGrp;
				di.bFixNum = grp.bFixNum;
				di.grpNum = grp.grpNum;
//				cout << "grpNum in init_semaphores DI " << di.grpNum << endl;
			}
		}
	}
	for (auto & ai : gDat.vecInitAI) {
		for (const auto & grp : gDat.vecInitGRP) {		// group id
			if (ai.id == grp.semaId) {
				ai.gId = grp.grpId;
				break;
			}
		}
		for (const auto & thr : gDat.vecInitlist) {		// threshold cover
			if (ai.id == thr.semaId) {
				ai.name = thr.name;
				if (ai.threshold_abs.size() > 0)
					ai.threshold_abs = thr.setVal;
				else if (ai.threshold_rel.size() > 0)
					ai.threshold_rel = thr.setVal;
				break;
			}
		}
		for (const auto & grp : gDat.vecInitGRP) {		// group data
			if (ai.id == grp.semaId) {
				ai.bGrp = grp.bGrp;
				ai.bFixNum = grp.bFixNum;
				ai.grpNum = grp.grpNum;
//				cout << "grpNum in init_semaphores AI " << ai.grpNum << endl;
			}
		}
	}
#endif
	return true;
}
bool chkInitThresholdDat() {
	string fn = gDat.exePath + string("cfg/") + string("init_list.csv");
	const char * pf = fn.c_str();
	FILE * stream;
	stream = fopen(pf, "r");
	if (stream == NULL) {
		stream = fopen(pf, "w");
		if (stream == NULL) {
			cout << "create file (init_list.csv) failed!\n";
			return false;
		} else {
			fclose(stream);
			vector<vector<string> > newDat;
			vector<string> tmp;
			tmp.push_back("devId");
			tmp.push_back("semaId");
			tmp.push_back("thrd");
			tmp.push_back("name");
			newDat.push_back(tmp);
			writeCsv(fn, newDat);
			return true;
		}
	}

	vector<vector<string> > dat;
	if (!readCsv(dat, fn))
		return false;
	if (dat.size() == 1)
		return true;
	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, fn, 3))
			return false;
		string sid = col[1];
		if (!chkFmtSemaId(sid)) {
			outputInitItm(col);
			cout << "wrong sema id !" << endl;
			return false;
		}
		string dId = col[0];
		u32 sz = dId.size();
		if ((sz != 6) && (sz != 7)) {
			outputInitItm(col);
			cout << "wrong format of semaphore's devId !" << endl;
			continue;
		}
	}
	return true;
}
bool chkDevGrpDat(const string & devName) {
	string p = gDat.exePath + string("cfg/grp/") + devName + string(".csv");
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return false;

	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 3))
			return false;
		string sid = col[0];
		if (!chkFmtSemaId(sid)) {
			outputInitItm(col);
			cout << "wrong sema id !" << endl;
			return false;
		}
		string gId = col[1];
		const char * s = gId.c_str();
		u32 sz = gId.size();
		if (sz == 7)
			gId = "0" + gId;
		if (!BInt::isIdAvailable(gId, 8)) {	// 非标准id, 判断有效性
			if (sz == 1) {
				if ((s[0] >= '0') && (s[0] <= '9')) {
					gId = string("0") + gId;
				} else {
					outputInitItm(col);
					cout << "unknown group Id format !" << endl;
					return false;
				}
			} else if (sz == 2) {
				if (!BInt::isIdAvailable(gId, 2)) {
					outputInitItm(col);
					cout << "wrong format of semaphore's group Id !" << endl;
					return false;
				}
			} else if (sz == 3) {
				if (!((s[0] == 'n') && ((s[1] >= '0') && (s[1] <= '9'))
						&& ((s[2] >= '0') && (s[2] <= '9')))) {
					outputInitItm(col);
					cout << "wrong format of semaphore's group Id !" << endl;
					return false;
				}
			} else {
				outputInitItm(col);
				cout << "unknown group Id format !" << endl;
				return false;
			}
		}
	}
	return true;
}
bool chkAllDeviceGrpDat() {
	string path = gDat.exePath + string("cfg/grp");
 	DIR *dp = opendir(path.c_str());
 	if (dp == NULL) {
 		cout << "can't find the path:" << path << endl;
 		return false;
 	}
	struct dirent *dirp ;
	while( ( dirp = readdir( dp ) ) != NULL) {
		if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0)
		  continue;
		int size = strlen(dirp->d_name);
		//如果是.csv文件，长度至少是5
		if(size < 5)
		  continue;
		//只存取.mp3扩展名的文件名
		if(strcmp( ( dirp->d_name + (size - 4) ) , ".csv") != 0)
		  continue;
		*(dirp->d_name +(size - 4)) = 0;
		if (!chkDevGrpDat(string(dirp->d_name)))
			return false;
	 };
	return true;
}
bool almTransChk(string & s) {
	if (s == "")
		return true;
	else if (s == "一级告警")
		s = "1";
	else if (s == "二级告警")
		s = "2";
	else if (s == "三级告警")
		s = "3";
	else if (s == "四级告警")
		s = "4";
	else
		return false;
	return true;

}
bool chkDevDI(vector<string> & col) {
	string aId = col[ADIO_Col::INIT_SEMA_AID];
	if (!aId.empty() && !chkFmtSemaId(aId)) {
		outputInitItm(col);
		cout << "wrong refer id !" << endl;
		return false;
	}
	const char * s;
	string logic = col[ADIO_Col::INIT_SEMA_LOGIC];
	if (logic.size() != 0) {
		s = logic.c_str();
		if ((*s == 'h') || (*s == 'H'))
			logic = string("h");
		else if ((*s == 'l') || (*s == 'L'))
			logic = string("l");
		else {
			outputInitItm(col);
			cout << "wrong format of DI's logic !" << endl;
			return false;
		}
	}
//	string backlash = col[ADIO_Col::INIT_SEMA_BACKLASH];
//	if (atof(backlash.c_str()) < 0) {
//		outputInitItm(col);
//		cout << "wrong format of AI's backlash !" << endl;
//		return false;
//	}
	string delayA = col[ADIO_Col::INIT_SEMA_A_ALM_DELAY];
	string delayB = col[ADIO_Col::INIT_SEMA_B_ALM_DELAY];
	string delayC = col[ADIO_Col::INIT_SEMA_C_ALM_DELAY];
	string delayD = col[ADIO_Col::INIT_SEMA_D_ALM_DELAY];
//	int dlyA = atoi(delayA.c_str());
//	int dlyB = atoi(delayB.c_str());
//	int dlyC = atoi(delayC.c_str());
//	int dlyD = atoi(delayD.c_str());
//	if ((dlyA < 0) || (dlyB < 0) || (dlyC < 0) || (dlyD < 0)) {
//		outputInitItm(col);
//		cout << "wrong format of DI's delay !" << endl;
//		return false;
//	}
	string thrA = col[ADIO_Col::INIT_SEMA_A_THRESHOLD];
	string thrB = col[ADIO_Col::INIT_SEMA_B_THRESHOLD];
	string thrC = col[ADIO_Col::INIT_SEMA_C_THRESHOLD];
	string thrD = col[ADIO_Col::INIT_SEMA_D_THRESHOLD];
	int tA = atoi(thrA.c_str());
	int tB = atoi(thrB.c_str());
	int tC = atoi(thrC.c_str());
	int tD = atoi(thrD.c_str());
	if ((tA < 0) || (tB < 0) || (tC < 0) || (tD < 0)) {
		outputInitItm(col);
		cout << "wrong format of DI's threshold !" << endl;
		return false;
	}
	string levelA = col[ADIO_Col::INIT_SEMA_A_ALM_CLASS];
	string levelB = col[ADIO_Col::INIT_SEMA_B_ALM_CLASS];
	string levelC = col[ADIO_Col::INIT_SEMA_C_ALM_CLASS];
	string levelD = col[ADIO_Col::INIT_SEMA_D_ALM_CLASS];
	if (!almTransChk(levelA) || !almTransChk(levelB)
			|| !almTransChk(levelC) ||!almTransChk(levelD)) {
			outputInitItm(col);
			cout << "wrong format of DI's alm level!" << endl;
			return false;
	}
	if ((levelA.size() != 0) || (levelB.size() != 0)
			|| (levelC.size() != 0) || (levelD.size() != 0)) {	// 是告警，否则是状态
		bool bAIDef = aId.size() > 0;
		bool bCompDef = logic.size() > 0;

		if ((bAIDef && !bCompDef) || (!bAIDef && bCompDef)) {
			outputInitItm(col);
			cout << "wrong configure of DI, miss sth." << endl;
			return false;
		}
	}

	return true;
}
bool less0(const string & s) {
	if (s.size() != 0) {
		if (atof(s.c_str()) <= 0) {
			return true;
		}
	}
	return false;
}
bool chkDevAI(vector<string> & col) {
	string absA= col[ADIO_Col::INIT_SEMA_A_ABS];
	string relA= col[ADIO_Col::INIT_SEMA_A_REL];
	string absB= col[ADIO_Col::INIT_SEMA_B_ABS];
	string relB= col[ADIO_Col::INIT_SEMA_B_REL];
	string absC= col[ADIO_Col::INIT_SEMA_C_ABS];
	string relC= col[ADIO_Col::INIT_SEMA_C_REL];
	string absD= col[ADIO_Col::INIT_SEMA_D_ABS];
	string relD= col[ADIO_Col::INIT_SEMA_D_REL];
	if (less0(absA) || less0(absB) || less0(absC) || less0(absD)
	 || less0(relA) || less0(relB) || less0(relC) || less0(relD)) {
		outputInitItm(col);
		cout << "wrong format of AI's Abs or Rel threshold !" << endl;
		return false;
	}

	return true;
}
bool chkDevSemaphores(const string & devName) {
	string p = gDat.exePath + string("cfg/base/") + devName + string(".csv");
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return false;

	for (u32 x = 1; x < dat.size(); x++) {
		vector<string> & col = dat[x];
		if (skipTheLine(col))
			continue;
		if (checkQuit(col, x, p, 28))
			return false;
		string sid = col[ADIO_Col::INIT_SEMA_ID];
		if (!chkFmtSemaId(sid)) {
			outputInitItm(col);
			cout << "wrong sema id !" << endl;
			return false;
		}
		switch(getSemaphoreType(sid)) {
		case T_SEMA_DI:
			if (!chkDevDI(col))
				return false;
			break;
		case T_SEMA_AI:
			if (!chkDevAI(col))
				return false;
			break;
		case T_SEMA_DO:
		case T_SEMA_AO:
			break;
		}
	}
	return true;
}
bool checkAllDeviceDat() {
	if (!initDev18Dat())
		return false;
	if(!load_init_devCommErrcfg())
		return false;
	string path = gDat.exePath + string("cfg/base");
 	DIR *dp = opendir(path.c_str());
 	if (dp == NULL) {
 		cout << "can't find the path:" << path << endl;
 		return false;
 	}
	struct dirent *dirp ;
	while( ( dirp = readdir( dp ) ) != NULL) {
		if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0)
		  continue;
		int size = strlen(dirp->d_name);
		//如果是.wav文件，长度至少是5
		if(size < 5)
		  continue;
		//只存取.mp3扩展名的文件名
		if(strcmp( ( dirp->d_name + (size - 4) ) , ".csv") != 0)
		  continue;
		*(dirp->d_name +(size - 4)) = 0;
		if (!chkDevSemaphores(string(dirp->d_name)))
			return false;
	 }
	return true;
}
bool chkInitDat() {
	if (!checkAllDeviceDat())
		return false;
	if (!chkAllDeviceGrpDat())
		return false;
	if (!chkInitThresholdDat())
		return false;
	return true;
}
bool init_productModelTbl() {
	string p = gDat.exePath + string("cfg/DevDefCfg.csv");
	vector<vector<string> > data;
	if (!readCsv(data, p))
		return false;
	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 12))
			return false;
		stDevDefCfgItm itm;
		itm.productModelIdx = data[x][0];
		itm.productModel_name = data[x][1];
		itm.devType = data[x][2];
		itm.format = data[x][3];
		itm.ver = data[x][4];
		itm.bufSize = atoi(data[x][5].c_str());
		if (itm.bufSize > MAX_BUF_SIZE) {
			cout << "init_productModelTbl(): buff size is  biger then " << MAX_BUF_SIZE << " ---> "
					<< itm.bufSize << endl;
			return false;
		}
		itm.baudrate = atoi(data[x][6].c_str());
		itm.databit = atoi(data[x][7].c_str());
		itm.parity = *(data[x][8].c_str());
		ostringstream buffer;
		float f = atof(data[x][9].c_str());
		buffer << f;
		itm.stopbit = buffer.str();
		itm.addr = data[x][10];
		itm.maxdelay = atoi(data[x][11].c_str());
		gDat.productDefCfgInitTbl.push_back(itm);
	}
	return init_protocol_parse_files();
}
bool init_protocol_parse_files() {
	for (const auto & i : gDat.productDefCfgInitTbl) {
		if (i.format.empty())	// 01 and 19 设备
			continue;
		if (i.productModel_name == "机房环境")
			continue;
		string parseFile = gDat.exePath + string("cfg/parse/") + i.productModel_name + string(".csv");
		PrtclDat * p = new PrtclDat;
		if (!p->initProtocolParseDat(parseFile)) {
			cout << "设备通信协议解析文件出错！" << endl;
			return false;
		}
		gDat.prtclDatTbl.push_back(p);
	}
	u8 dev18Def = gDat.gatherersInstallDat8;
	u8 mask = 1;
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
	for (u32 i = 0; i < 8; ++i) {
		mask = (1 << i);
		if ((dev18Def & mask) > 0) {
			string parseFile = gDat.exePath + string("cfg/parse/") + pre + "机房环境";
			char ch[3] = {'0',0,0};
			ch[1] = '0' + i + 1;
			parseFile = parseFile + string(ch) + string(".csv");
			PrtclDat * p = new PrtclDat;
			if (!p->initProtocolParseDat(parseFile)) {
				cout << "设备通信协议解析文件出错！" << endl;
				return false;
			}
			gDat.prtclDatTbl.push_back(p);
		}
	}
	return true;
}
//bool init_DevCommCfg() {
//	if (!init_productModelTbl())
//		return false;
//
//	string p = gDat.exePath + string("cfg/DevCommCfg.csv");
//	vector<vector<string> > data;
//	if (!readCsv(data, p))
//		return false;
//	for (u32 x = 1; x < data.size(); x++) {
//		if (skipTheLine(data[x]))
//			continue;
//		if (checkQuit(data[x], x, p, 11))
//			return false;
//
//		stDevInitCommCfg itm;
//		itm.Id = data[x][0];
//		itm.Code = data[x][1];
//		bool find = false;
//		for (const auto i : gDat.productModelTbl) {
//			if (data[x][2] == i.strIdx) {
//				find = true;
//				itm.productModelIdx = data[x][2];
//				break;
//			}
//		}
//		if (!find) {
//			cout << "can't find product model idx in DevProtocolTypeIdx.csv file\n";
//			return false;
//		}
//		itm.portType = data[x][3];
//		itm.portIdx = data[x][4];
//		itm.baudrate = atoi(data[x][5].c_str());
//		itm.databit = atoi(data[x][6].c_str());
//		itm.parity = *(data[x][7].c_str());
//		ostringstream buffer;
//		float f = atof(data[x][8].c_str());
//		buffer << f;
//		itm.stopbit = buffer.str();
//		itm.addr = data[x][9];
//		itm.maxdelay = atoi(data[x][10].c_str());
//		gDat.devCommCfgTbl.push_back(itm);
//	}
//
//	return true;
//}
bool init_prtclFmt() {
	vector<vector<string> > data;
	data.clear();
	string p = gDat.exePath + string("cfg/prtclFmt.csv");
	if (!readCsv(data, p))
		return false;
	gDat.prtclFmtTbl.clear();
	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 4))
			return false;
		protocolFormat it;
		it.type = data[x][0];
		it.soi = data[x][1];
		it.eoi = data[x][2];
		it.code = data[x][3];
		gDat.prtclFmtTbl.push_back(it);
	}
	return true;
}
bool init_so_file() {
	if (!init_prtclFmt())
		return false;
	cout << "-------- init so file: --------\n";
	for(const auto & i : gDat.prtclFmtTbl){
		ProtocolParse * p = new ProtocolParse(i);
		if (!p->valid())		// create fault.
			return false;
		gDat.vecPPP.push_back(p);

	}
	return true;
}
bool init_portCfg() {
	vector<vector<string> > data;
	data.clear();
	string p = gDat.exePath + string("cfg/port.conf");
	if (!readCsv(data, p))
		return false;
	gDat.portCfgTbl.clear();
	for (u32 x = 1; x < data.size(); x++) {
		if (skipTheLine(data[x]))
			continue;
		if (checkQuit(data[x], x, p, 3))
			return false;
		portConfigItm it;
		it.ttyType = data[x][0];
		it.ttyPort = data[x][1];
		it.devId = data[x][2];
		gDat.portCfgTbl.push_back(it);

		devIdCode idcd;
		idcd.Code = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +  it.devId;
		idcd.Id = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +  it.devId;
		gDat.vecDev.push_back(idcd);
	}
	return true;
}

/*
,,说明：十位数1：有返回；,2返回数据符合,,,,,,
,,,,,,,,个位数0、1、2、3,。。。发送优先级（从高到低）至少发两次

*/
bool init_DevScanTbl() {
	string p = gDat.exePath + string("cfg/DevScan.csv");
	vector<vector<string> > dat;
	if (!readCsv(dat, p))
		return false;
	u32 sz = dat.size();
	if (sz <= 1 ) {
		cout << "empty data -> " << p << endl;
		return true;
	}
	portScanTblItm lastPortItm;
	lastPortItm.devType = -1;

	devIdentifyItm lastDevItm;
	lastDevItm.devModel = -1;

	for (u32 x = 1; x < sz; x++) {
		if (skipTheLine(dat[x]))
			continue;
		if (checkQuit(dat[x], x, p, 7))
			return false;

		vector<string> & col = dat[x];
		string devType = col[0];
		string model = col[1];
		string way = col[2];
		string outStr = col[3];
		string content = col[4];
		string rtnLen = col[5];
		string pos = col[6];


		if (!devType.empty() && (atoi(devType.c_str()) > 0)) {
			if ((atoi(devType.c_str()) != lastPortItm.devType)
					&& (lastPortItm.devType != -1)) {
				if (!model.empty() && !way.empty() && !outStr.empty()) {
					if ((atoi(model.c_str()) != lastDevItm.devModel)
							&&(lastDevItm.devModel != -1)) {
#if DEBUG_INIT_DAT
						cout << "\t[-]add new device(" << lastDevItm.devModel << ") scan item\n";
#endif
						lastPortItm.identifyTbl.push_back(lastDevItm);
						lastDevItm.devModel = -1;
						lastDevItm.ids.clear();
					}
				}
#if DEBUG_INIT_DAT
				cout << "add new device Type(" << lastPortItm.devType << ")\n";
#endif
				gDat.portDevScanTbl.push_back(lastPortItm);
				lastPortItm.devType = -1;
				lastPortItm.identifyTbl.clear();
			}
#if DEBUG_INIT_DAT
			cout << "-------------------------------------------------------\n";
			cout << "set new Device Type(" << devType << ")\n";
#endif
			lastPortItm.devType = atoi(devType.c_str());
		}

		if (!model.empty() && !way.empty() && !outStr.empty()) {
			if ((atoi(model.c_str()) != lastDevItm.devModel)
					&&(lastDevItm.devModel != -1)) {
#if DEBUG_INIT_DAT
				cout << "\t[-]add new device(" << lastDevItm.devModel << ") scan item\n";
#endif
				lastPortItm.identifyTbl.push_back(lastDevItm);
				lastDevItm.devModel = -1;
				lastDevItm.ids.clear();
			}
#if DEBUG_INIT_DAT
			cout << "\t[+]set new device(" << model << ") scan item\n";
#endif
			lastDevItm.devModel = (u32)atoi(model.c_str());
			lastDevItm.way = (u32)atoi(way.c_str());
			lastDevItm.outStr = outStr;
			if (!pos.empty())
				lastDevItm.startPos = (u32)atoi(pos.c_str());
			else
				lastDevItm.startPos = 0;
			if (!rtnLen.empty())
				lastDevItm.rtnLen = (u32)atoi(rtnLen.c_str());
			else
				lastDevItm.rtnLen = 0;
		}

		if (model.empty() && way.empty() && outStr.empty() && rtnLen.empty()
				&& !pos.empty()) {
#if DEBUG_INIT_DAT
			cout << "\t\tnew sub itm\n";
#endif
			lastDevItm.ids.push_back(
				make_pair(content, (u32)atoi(pos.c_str())));
		}
	}

	lastPortItm.identifyTbl.push_back(lastDevItm);
#if DEBUG_INIT_DAT
	cout << "\t[-]add new device(" << lastDevItm.devModel << ") scan item\n";
#endif
	gDat.portDevScanTbl.push_back(lastPortItm);

#if DEBUG_INIT_DAT
	cout << "************************************************************************\n";
	cout << "[devType]\nmodel\tfunc\toutStr\t\t\tcontent\t\t\trtnLen\tpos\n";
#endif
#if DEBUG_INIT_DAT
	for (const auto & i : gDat.portDevScanTbl) {
		cout << "[Device Type = " << i.devType << "]\n";
		cout << "-------------------------------------------------------------------------\n";
#endif
#if DEBUG_INIT_DAT
		for (const auto &j : i.identifyTbl) {
			cout << j.devModel << "\t" << j.way << "\t" << j.outStr;
			cout << "\t\t\t\t" << j.rtnLen << "\t" << j.startPos << endl;
#endif
#if DEBUG_INIT_DAT
			for (const auto &k : j.ids) {
				cout << "\t\t\t\t\t" << k.first << "\t\t\t" << k.second << endl;
			}
		}
	}
#endif
#if DEBUG_INIT_DAT
	cout << "************************************************************************\n";
#endif
	return true;
}
#if 0
bool init_DevCommCfg() {
	string p = gDat.exePath + string("cfg/DevComDat.xml");
	xmlDoc *pDoc = xmlReadFile(p.c_str(), NULL, 0);
	if (pDoc == NULL) {
		printf("error: could not handle file %s\n", p.c_str());
		return false;
	} else {
		const xmlChar * xpath = (const xmlChar *) ("//Device");
		xmlXPathObjectPtr rst = getNodeset(pDoc, xpath);
		if (rst) {
			stInitComm itm;
			xmlNodeSetPtr nodeset = rst->nodesetval;
			xmlNodePtr cur;

			for (int i = 0; i < nodeset->nodeNr; i++) { // device number
				cur = nodeset->nodeTab[i];
				if (cur->type != XML_ELEMENT_NODE)
					continue;
				xmlAttr * attr = cur->properties;
//				const xmlChar * id;
//				const xmlChar * code;
				if (attr) {
					if (!xmlStrcmp(attr->name, (const xmlChar *) "Id")) {
						itm.id = string((char *) attr->children->content);
						itm.id = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + itm.id;
						attr = attr->next;
						if (attr) {
							if (!xmlStrcmp(attr->name,
									(const xmlChar *) "Code")) {
								itm.code = (char *) attr->children->content;
								itm.code = gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +itm.code;
							} else {
								cout << "No \"Code\" id in DevComDat.xml file !"
										<< endl;
								return false;
							}
						}
					} else {
						cout << "No \"Id\" id in DevComDat.xml file !" << endl;
						return false;
					}
				}
				xmlNodePtr subNode;
				for (subNode = cur->children; subNode; subNode =
						subNode->next) {
					if (subNode->type == XML_ELEMENT_NODE) {
						if (!xmlStrcmp(subNode->name,
								(const xmlChar *) "CommPrtcl")) {
							xmlAttr * attr = subNode->properties;
							if (attr) {
								while (attr) {
#if DEBUG_INIT_DAT
									printf(" %s=\"%s\" \n", attr->name,
											attr->children->content);
#endif
									char * sVal =
											(char *) attr->children->content;
									if (!xmlStrcmp(attr->name,
											(const xmlChar *) "PortType")) {
										itm.portType = (char *) sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "PortIdx")) {
										itm.portIdx = (char *) sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "BaudRate")) {
										itm.baudrate = atoi(sVal);
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "DataBit")) {
										itm.databit = atoi(sVal);
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "Parity")) {
										string s = string(sVal);
										if (s == string("奇校验"))
											itm.parity = 'O';
										else if (s == string("偶校验"))
											itm.parity = 'E';
										else
											itm.parity = 'N';
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "StopBit")) {
										ostringstream buffer;
										float f = atof(sVal);
										buffer << f;
										itm.stopbit = buffer.str();
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "MaxDelay")) {
										itm.maxdelay = atoi(sVal);
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "type")) {
										itm.type = (char *) sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "format")) {
										itm.format = (char *) sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "ver")) {
										itm.ver = (char *) sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "addr")) {
//										itm.addr = atoi(sVal);
										itm.addr = sVal;
									} else if (!xmlStrcmp(attr->name,
											(const xmlChar *) "bufSize")) {
										itm.bufSize = atoi(sVal);
										if (itm.bufSize > MAX_BUF_SIZE) {
											cout << "buff size is  biger then " << MAX_BUF_SIZE << " ---> "
													<< itm.bufSize << endl;
											return false;
										}
									}
									attr = attr->next;
								}
							}

						}
					}
				}
				gDat.vecDevInitComm.push_back(itm);
			}
			xmlXPathFreeObject(rst);
		} else {
			cout << "\n\n\n !!! no device defined in " << p.c_str() << endl << endl << endl;
//			return false;
		}
	}
	return true;
}
#endif
//void clearInitDat() {
//	gDat.vecInitlist.clear();
//	gDat.vecInitDI.clear();
//	gDat.vecInitAI.clear();
//	gDat.vecInitDO.clear();
//	gDat.vecInitAO.clear();
//	gDat.vecInitGRP.clear();
//}
void getDevsIdsByteType (const string & devType, vector<string> &vec) {
	for (const auto &dev : gDat.vecDev) {
		string out;
//		getDevTypeStr(	dev.Id,	out);
		getDevTypeStr_fullId(	dev.Id,	out);
		if (out == devType)
			vec.push_back(dev.Id);
	}
}
const string & find1stDevIdByType(const string & devType) {
	vector<string> vec;
	getDevsIdsByteType(devType, vec);
	if (vec.size() > 0)
		return vec.at(0);
	else
		return gDat.emptyStr;
}

GlobalDat::~GlobalDat() {
	WHERE_AM_I
	ClearPointerVector(vecPPP);
	ClearPointerVector(prtclDatTbl);
//	clearVector(vecDevComm);
}
//const stInitSemaphore2 * getSemaCfg(string id) {
//	for (const auto & cfg : gDat.vecInitSema2) {
//		if (id == cfg.id) {
//			return &cfg;
//		}
//	}
//	return NULL;
//}
//string findGrpIdReferToMe(string myId) {
////	for (const auto & cfg : gDat.vecInitSema2) {
////		if (myId == cfg.grpId) {
////			return cfg.id;
////		}
////	}
//	return string("");
//}
#if 1
#endif
#if 0
string Trim(string& str) {
	str.erase(0, str.find_first_not_of(" \t\r\n"));
	str.erase(str.find_last_not_of(" \t\r\n") + 1);
	return str;
}
void readCsv(vector<vector<string> > & data, string & filename) {
	ifstream fin(filename.c_str());

	string line;
	while (getline(fin, line)) {
		//cout << line << endl;

		istringstream sin(line);
		vector<string> fields;
		string field;
		while (getline(sin, field, ',')) {
			fields.push_back(field);
#if DEBUG_CSV
			cout << field << ",";
#endif
		}
#if DEBUG_CSV
		cout << endl;
#endif
		data.push_back(fields);
//		string name = Trim(fields[0]);
//		string age = Trim(fields[1]);
//		string birthday = Trim(fields[2]);
//		cout << name << "\t" << age << "\t" << birthday << endl;
	}
}
#endif
#if 0
#endif
