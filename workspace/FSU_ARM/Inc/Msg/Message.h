#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "define.h"
#include <string>
#include <libxml/tree.h>

using namespace std;
/*     !!! 以下3个定义顺序固定，不可修改  !!!  */

static string MsgName[30] = {
	"LOGIN",                //SC<—FSU//注册
	"LOGIN_ACK",            //SC—>FSU//注册响应
	"LOGOUT",               //SC<—FSU//登出
	"LOGOUT_ACK",           //SC—>FSU//登出响应
	"GET_DATA",             //SC—>FSU//用户请求监控点数据
	"GET_DATA_ACK",         //SC<—FSU//用户请求监控点数据响应
	"GET_HISDATA",          //SC—>FSU//用户请求监控点历史数据
	"GET_HISDATA_ACK",      //SC—>FSU//用户请求监控点历史数据响应
	"SEND_ALARM",           //SC<—FSU//实时告警发送
	"SEND_ALARM_ACK",       //SC—>FSU//实时告警发送确认
	"SET_POINT",            //SC—>FSU//写数据请求
	"SET_POINT_ACK",        //SC<—FSU//写数据响应
	"TIME_CHECK",           //SC—>FSU//发送时钟消息
	"TIME_CHECK_ACK",       //SC<—FSU//时钟同步响应
	"GET_LOGININFO",        //SC—>FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
	"GET_LOGININFO_ACK",    //SC<—FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
	"SET_LOGININFO",        //SC—>FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
	"SET_LOGININFO_ACK",    //SC<—FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
	"GET_FTP",              //SC—>FSU//获取FSU的FTP用户、密码数据
	"GET_FTP_ACK",          //SC<—FSU//获取FSU的FTP用户、密码数据响应
	"SET_FTP",              //SC—>FSU//设置FSU的FTP用户、密码数据
	"SET_FTP_ACK",          //SC<—FSU//设置FSU的FTP用户、密码数据响应
	"GET_FSUINFO",          //SC—>FSU//获取FSU的状态参数
	"GET_FSUINFO_ACK",      //SC<—FSU//获取FSU的状态参数响应
	"SET_FSUREBOOT",        //SC—>FSU//重启FSU
	"SET_FSUREBOOT_ACK",    //SC<—FSU//重启FSU响应
	"GET_THRESHOLD",        //SC—>FSU//用户请求监控点门限数据
	"GET_THRESHOLD_ACK",    //SC<—FSU//用户请求监控点门限数据响应
	"SET_THRESHOLD",        //SC—>FSU//用户请求写监控点门限数据请求
	"SET_THRESHOLD_ACK"     //SC<—FSU//用户请求写监控点门限数据响应
};
static string MsgId[30] = {
	"101",  //SC<—FSU//注册
	"102",  //SC—>FSU//注册响应
	"103",  //SC<—FSU//登出
	"104",  //SC—>FSU//登出响应
	"401",  //SC—>FSU//用户请求监控点数据
	"402",  //SC<—FSU//用户请求监控点数据响应
	"403",  //SC—>FSU//用户请求监控点历史数据
	"404",  //SC<—FSU//用户请求监控点历史数据响应
	"501",  //SC<—FSU//实时告警发送
	"502",  //SC—>FSU//实时告警发送确认
	"1001", //SC—>FSU//写数据请求
	"1002", //SC<—FSU//写数据响应
	"1301", //SC—>FSU//发送时钟消息
	"1302", //SC<—FSU//时钟同步响应
	"1501", //SC—>FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
	"1502", //SC<—FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
	"1503", //SC—>FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
	"1504", //SC<—FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
	"1601", //SC—>FSU//获取FSU的FTP用户、密码数据
	"1602", //SC<—FSU//获取FSU的FTP用户、密码数据响应
	"1603", //SC—>FSU//设置FSU的FTP用户、密码数据
	"1604", //SC<—FSU//设置FSU的FTP用户、密码数据响应
	"1701", //SC—>FSU//获取FSU的状态参数
	"1702", //SC<—FSU//获取FSU的状态参数响应
	"1801", //SC—>FSU//重启FSU
	"1802", //SC<—FSU//重启FSU响应
	"1901", //SC—>FSU//用户请求监控点门限数据
	"1902", //SC<—FSU//用户请求监控点门限数据响应
	"2001", //SC—>FSU//用户请求写监控点门限数据请求
	"2002"  //SC<—FSU//用户请求写监控点门限数据响应
};
//enum class EPkIdx  {
//	PK_IDX_101_LOGIN 				,
//	PK_IDX_102_LOGIN_ACK 			,
//	PK_IDX_103_LOGOUT				,
//	PK_IDX_104_LOGOUT_ACK			,
//	PK_IDX_401_GET_DATA				,
//	PK_IDX_402_GET_DATA_ACK			,
//	PK_IDX_403_GET_HISDATA			,
//	PK_IDX_404_GET_HISDATA_ACK		,
//	PK_IDX_501_SEND_ALARM			,
//	PK_IDX_502_SEND_ALARM_ACK		,
//	PK_IDX_1001_SET_POINT			,
//	PK_IDX_1002_SET_POINT_ACK		,
//	PK_IDX_1301_TIME_CHECK			,
//	PK_IDX_1302_TIME_CHECK_ACK		,
//	PK_IDX_1501_GET_LOGININFO		,
//	PK_IDX_1502_GET_LOGININFO_ACK	,
//	PK_IDX_1503_SET_LOGININFO		,
//	PK_IDX_1504_SET_LOGININFO_ACK	,
//	PK_IDX_1601_GET_FTP				,
//	PK_IDX_1602_GET_FTP_ACK			,
//	PK_IDX_1603_SET_FTP				,
//	PK_IDX_1604_SET_FTP_ACK			,
//	PK_IDX_1701_GET_FSUINFO			,
//	PK_IDX_1702_GET_FSUINFO_ACK		,
//	PK_IDX_1801_SET_FSUREBOOT		,
//	PK_IDX_1802_SET_FSUREBOOT_ACK	,
//	PK_IDX_1901_GET_THRESHOLD		,
//	PK_IDX_1902_GET_THRESHOLD_ACK	,
//	PK_IDX_2001_SET_THRESHOLD		,
//	PK_IDX_2002_SET_THRESHOLD_ACK
//};
#define STRING(x) #x
#define BLANK(nd) \
		if (nd->type != XML_ELEMENT_NODE) nd = nd->next
#define NODE_VERIFY(nd)\
		if (nd == NULL)	goto FAILED
#define IF_NODE_IS(nd, str)\
		BLANK(nd);\
		if (!xmlStrcmp(nd->name, (unsigned char *) STRING(str)))
struct MsgHead { // PK_TYPE部分
private:
	bool getMsgId (u32 idx, string& out);
	bool getMsgName (u32 idx, string & out);
public:
	MsgHead():str(""), id(""){}
	MsgHead(u32 msgIdx) {
		getMsgName(msgIdx, str);
		getMsgId(msgIdx, id);
	}
	string str;
	string id;
};

class MessagePair {
	friend class SCCallProxy;
public:
	MessagePair(u32 inMsgIdx, u32 outMsgIdx,bool b) : _in(inMsgIdx),_out(outMsgIdx), _bReq2SC(b) {};
	virtual ~MessagePair(){}
	int makeResponseStr(string &xmlStr, string &str2SC){
		return makeOutXml(parseXmlInfoPart(xmlStr), str2SC);
	}
	void makeOutXmlHead(xmlDocPtr &doc, xmlNodePtr &root, xmlNodePtr &info);
	void wrtXmlToStr(xmlDocPtr &doc, string &str2SC);
private:
	virtual int makeOutXml(bool bSuccess, string &str2SC){return 0;}
	virtual int parseXmlInfoPart(string &xmlStr){return 0;}
protected:
	MsgHead _in;
	MsgHead _out;
	bool	_bReq2SC;
};
class SCCallProxy {
public:
	SCCallProxy();
	~SCCallProxy();
	int makeOutXml(u32 msgIdx, string &str2SC);
	int parseSCResponse(u32 msgIdx, string &strSCRes);
	void getLoginPak(string & str2SC);
	void getLogoutPak(string & str2SC);
	int getAlarmPak(string & str2SC);
private:
	MessagePair * pLoginAck;
	MessagePair * pLogoutAck;
	MessagePair * pSendAlarmAck;
};
class MessageMgr {
private:
	static bool getMsgIdxByIdStr(string s, u32 & out){
		unsigned int sz = countof(MsgId);
		for (unsigned int idx = 0; idx < sz; ++idx) {
			if (s == MsgId[idx]) {
				out = idx;
				return true;
			}
		}
		return false;
	}
	static bool isMsgHeadTypeAvail(MsgHead & mh, s32 &oIdx) { //Name,Code ok?
		u32 idx = -1;
		if (getMsgIdxByIdStr(mh.id, idx)
		  && (idx <  countof(MsgName))
		  && (MsgName[idx] == mh.str)) {
			oIdx = idx;
			return true;
		} else {
			oIdx  = -1;
			return false;
		}
	}
	static bool isMsgHeadAvail(string &xmlStr, s32 &idx);
	static int makeResponseStr(u32 msgIdx, string &xmlStr, string &str2SC);
public:
	static int hdlScResponse(string &xmlData);
	static int inComeMsgHdl(string &xmlData, string &str2SC);
};

#endif

