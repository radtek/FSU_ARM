/*
 * AppConfig.h
 *
 *  Created on: 2016-3-31
 *      Author: vmuser
 */

#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#include "define.h"
#include <string>
#include <vector>
#include "B_Interface.h"
#include "DevComm.h"
#include "PrtclDat.h"

using namespace std;
using namespace BInt;

const u32 GIDX_USERNAME 	= 0;
const u32 GIDX_PASSWORD  	= 1;
const u32 GIDX_FSUID  		= 2;
const u32 GIDX_FSUCODE  	= 3;
const u32 GIDX_FSUIP  		= 4;
const u32 GIDX_FSUCOMMTYPE	= 5;
const u32 GIdX_FSU_MODE		= 6;

const u32 GIDX_FTP_USERNAME	= 7;
const u32 GIDX_FTP_PASSWORD	= 8;
const u32 GIDX_IPSECUSER 	= 9;
const u32 GIDX_IPSECPWD	 	= 10;
const u32 GIDX_IPSECIP	 	= 11;
const u32 GIDX_SCIP_LOGIN	= 12;
const u32 GIDX_SCIP_GETTER	= 13;
const u32 GIDX_SCPORT		= 14;
const u32 GIdX_SC_MODE		= 15;
const u32 GIDX_SITE_CODE	= 16;
const u32 GIDX_SITE_TYPE	= 17;
const u32 GIDX_BAUDRATE_485	= 18;
//-------------------------------
const u32 GIDX_MAX_SIZE		= 19;

const u32 MAX_BUF_SIZE		= 8192;

const size_t gArrayIdxLen[GIDX_MAX_SIZE] = {
	USER_LENGTH,		// 0
	PASSWORD_LEN,		// 1
	FSUID_LEN,			// 2
	FSUID_LEN,			// 3
	IP_LENGTH,			// 4
	FSU_COMMTYPE_LEN,	// 5
	MODE_LENGTH,		// 6

	USER_LENGTH,		// 7
	PASSWORD_LEN,		// 8
	USER_LENGTH,		// 9
	PASSWORD_LEN,		//10
	IP_LENGTH,			//11
	IP_LENGTH,			//12
	IP_LENGTH,			//13
	PORT_LENGTH,		//14
	MODE_LENGTH,		//15
	SITECODE_LENGTH,	//16
	SITETYPE_LENGTH,	//17
	BAUD_LEN			//18
};

enum ADIO_Col {
	INIT_SEMA_NAME = 0,
	INIT_SEMA_ID,
	INIT_SEMA_UNIT,
	INIT_SEMA_ALMDESC1,
	INIT_SEMA_ALMDESC0,
	INIT_SEMA_AID,
	INIT_SEMA_LOGIC,
	INIT_SEMA_BACKLASH,
	INIT_SEMA_A_ALM_CLASS,
	INIT_SEMA_A_THRESHOLD,
	INIT_SEMA_A_ALM_DELAY,
	INIT_SEMA_A_ABS,
	INIT_SEMA_A_REL,
	INIT_SEMA_B_ALM_CLASS,
	INIT_SEMA_B_THRESHOLD,
	INIT_SEMA_B_ALM_DELAY,
	INIT_SEMA_B_ABS,
	INIT_SEMA_B_REL,
	INIT_SEMA_C_ALM_CLASS,
	INIT_SEMA_C_THRESHOLD,
	INIT_SEMA_C_ALM_DELAY,
	INIT_SEMA_C_ABS,
	INIT_SEMA_C_REL,
	INIT_SEMA_D_ALM_CLASS,
	INIT_SEMA_D_THRESHOLD,
	INIT_SEMA_D_ALM_DELAY,
	INIT_SEMA_D_ABS,
	INIT_SEMA_D_REL
};
static const string DI_STR = string("2");
static const string AI_STR = string("3");
static const string DO_STR = string("4");
static const string AO_STR = string("5");
struct commDevSemeIdAvailItm {
	string modelName;
	vector<string> ids;
};
struct st_args {
	bool en_xml = false;
	bool en_alm = false;
	bool en_com = false;
	bool en_parse = false;
	bool en_xml_cr = false;
};
struct GlobalDat {
	~GlobalDat();
	string cfgs[GIDX_MAX_SIZE];
	string myIp;
//	char UserName[USER_LENGTH];
//	char Password[PASSWORD_LEN];
//
//	char FsuId[FSUID_LEN];
//	char FsuCode[FSUID_LEN];
//	char FsuIP[IP_LENGTH];
//
//	char FTP_UserName	[USER_LENGTH];
//	char FTP_Password	[PASSWORD_LEN];
//
//	char IPSecUser	[USER_LENGTH];
//	char IPSecPWD	[PASSWORD_LEN];
//	char IPSecIP	[IP_LENGTH];
//	char SCIP		[IP_LENGTH];

	EnumRightMode RightLevel;
	//string RightLevel;
//	bool	bLogin;
	int netMode = 0;
	unsigned char gatherersInstallDat8;
	string	cpuOccRate;
	string	memOccRate;
	u32 rebootFlag;
	u32 baudRate485;
	vector<devIdCode> vecDev;
//	vector <stInit4ADIO>  vecInitlist;
//	vector <stInitDI>  vecInitDI;
//	vector <pair<string,string>> vvDIsName;
//	vector <stInitAI>  vecInitAI;
//	vector <pair<string,string>> vvAIsName;
//	vector <stInitDO>  vecInitDO;
//	vector <stInitAO>  vecInitAO;
//	vector <stInitGRP>  vecInitGRP;
	vector <pair<string, string>> vecDevCommErrCfg;
//	string devId_of_type16;
//	string devId_of_type17;
//	string devId_of_type18;
//	string devId_of_type19;
//	string devId_of_type02;
//	string devId_of_type06;
//	string devId_of_type07;
	string siteType; // A|B|C|D
	u32	thresholdSavePosOfDiFile;
	string semaIdPreString;
	string invalidStr;
	string validStr;
	string datType_DI;
	string datType_AI;
	string datType_DO;
	string datType_AO;
	string almLevelStr[4];
	string almFlag[2];
	string exePath;
	// pointer must be free !
	vector <DevComm *> vecDevComm;
	const string emptyStr = string("");
	string SCLoginIP;
	string SCGetterIP;
	const char * szSCLoginIP;
	const char * szSCGetterIP;
	int port;
	vector<portScanTblItm> portDevScanTbl;
	vector<portConfigItm> portCfgTbl;
	vector<portConfigItm> portCfgTbl_dev18;
	vector<protocolFormat> prtclFmtTbl;
	vector<ProtocolParse *> vecPPP;
	vector <stDevDefCfgItm>	productDefCfgInitTbl;
	vector <PrtclDat *> prtclDatTbl;
	vector <DevInit *> devInitTbl;
	vector <commDevSemeIdAvailItm> commDevSIdAvailTbl;
	vector <dev18CfgItm> dev18CfgTbl;
	vector <extSemaDefItm> extSemaDefTbl;
	vector <pair<int,int>> doorMagDefs;
	pair<int,int> infraDef;
	cameraRelDevs cameraDat;
	st_args args;
};

char * get_exe_path( char * buf, int count);
bool setConfig(u32 idx, string s, bool isInit);
//string findGrpIdReferToMe(string myId);
bool initConfig();
bool chkInitDat();
void clearInitDat();
bool init_semaphores();
//bool init_DevCommCfg();
bool init_DevScanTbl();
bool init_productModelTbl();
bool init_protocol_parse_files();
bool init_so_file();
bool init_portCfg();
void getDevsIdsByteType (const string & devType, vector<string> &vec);
const string & find1stDevIdByType(const string & devType);
void setSemaAvailable(string &id);
void getOccRate(string & c, string &m);
u32 getRebootFlag();
void setRebootFlag();
#endif /* APPCONFIG_H_ */
