#include <iostream>
#include <sstream>

#include "debug.h"
#include "Message.h"
#include "xmlHdl.h"
// SC -> FSU
#include "MsgTimeCheckRtnDat.h"
#include "MsgGetDatRtnDat.h"
#include "MsgGetHisDatRtnDat.h"
#include "MsgSetPointRtnDat.h"
#include "MsgGetThresholdRtnDat.h"
#include "MsgGetFtpRtnDat.h"
#include "MsgSetFtpRtnDat.h"
#include "MsgGetFsuInfoRtnDat.h"
#include "MsgSetFsuRebootRtnDat.h"
#include "MsgGetThresholdRtnDat.h"
#include "MsgSetThresholdRtnDat.h"
#include "MsgGetFsuLoginInfoRtnDat.h"
#include "MsgSetFsuLoginInfoRtnDat.h"
// FSU -> SC
#include "MsgLoginAck.h"
#include "MsgLogoutAck.h"
#include "MsgSendAlarmAck.h"

using namespace std;

//-------------------------------------------------------
bool MsgHead::getMsgId(u32 idx, string& out) {
	if (idx < countof(MsgId)) {
		out = MsgId[(unsigned int) idx];
		return true;
	}
	return false;
}
bool MsgHead::getMsgName(u32 idx, string & out) {
	if (idx < countof(MsgName)) {
		out = MsgName[(unsigned int) idx];
		return true;
	}
	return false;
}
SCCallProxy scProxy;
extern GlobalDat gDat;
SCCallProxy::SCCallProxy() {
	pLoginAck = new MsgLoginAck(PK_IDX_102_LOGIN_ACK, PK_IDX_101_LOGIN);
	pLogoutAck = new MsgLogoutAck(PK_IDX_104_LOGOUT_ACK, PK_IDX_103_LOGOUT);
	pSendAlarmAck = new MsgSendAlarmAck(PK_IDX_502_SEND_ALARM_ACK, PK_IDX_501_SEND_ALARM);
}
SCCallProxy::~SCCallProxy() {
	if (pLoginAck)
		delete pLoginAck;
	if (pLogoutAck)
		delete pLogoutAck;
	if (pSendAlarmAck)
		delete pSendAlarmAck;
}
int SCCallProxy::makeOutXml(u32 msgIdx, string &str2SC) {
	switch (msgIdx) {
	// out msg
	case PK_IDX_102_LOGIN_ACK:
		pLoginAck->makeOutXml(true, str2SC);
		break;
	case PK_IDX_104_LOGOUT_ACK:
		pLogoutAck->makeOutXml(true, str2SC);
		break;
	case PK_IDX_502_SEND_ALARM_ACK:
		pSendAlarmAck->makeOutXml(true, str2SC);
		break;
	default:
		return 0;
	}
	return 1;
}
int SCCallProxy::parseSCResponse(u32 msgIdx, string &strSCRes) {
	switch (msgIdx) {
	// income response
	case PK_IDX_102_LOGIN_ACK:
		return pLoginAck->parseXmlInfoPart(strSCRes);
		break;
	case PK_IDX_104_LOGOUT_ACK:
		return pLogoutAck->parseXmlInfoPart(strSCRes);
		break;
	case PK_IDX_502_SEND_ALARM_ACK:
		return pSendAlarmAck->parseXmlInfoPart(strSCRes);
		break;
	}
	return 0;
}
void SCCallProxy::getLoginPak(string & str2SC) {
	makeOutXml(PK_IDX_102_LOGIN_ACK, str2SC);
}
void SCCallProxy::getLogoutPak(string & str2SC) {
	makeOutXml(PK_IDX_104_LOGOUT_ACK, str2SC);
}
int SCCallProxy::getAlarmPak(string & str2SC) {
	return makeOutXml(PK_IDX_502_SEND_ALARM_ACK, str2SC);
}

//----------------------------------------------------------
void MessagePair::makeOutXmlHead(xmlDocPtr &doc, xmlNodePtr &root, xmlNodePtr &info) {
	doc = xmlNewDoc(BAD_CAST "1.0");
	string rootStr = _bReq2SC ? "Request" : "Response";
	root = xmlNewNode(NULL, BAD_CAST rootStr.c_str());
	xmlDocSetRootElement(doc, root);
	xmlNodePtr head = xmlNewChild(root, NULL, BAD_CAST "PK_Type", NULL);
	string name = _out.str;//_bReq2SC ? _out.str : _in.str;
	string id = _out.id;//_bReq2SC ? _out.id : _in.id;
	xmlNewChild(head, NULL, BAD_CAST "Name", BAD_CAST name.c_str());
	xmlNewChild(head, NULL, BAD_CAST "Code", BAD_CAST id.c_str());
	info = xmlNewChild(root, NULL, BAD_CAST "Info", BAD_CAST NULL);
}
void MessagePair::wrtXmlToStr(xmlDocPtr &doc, string &str2SC) {
	xmlChar *xmlbuff;
	int buffersize;
	xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &buffersize, NULL,
			(int)gDat.args.en_xml_cr);	//0 : no blank.
	str2SC = string((const char *)xmlbuff);
//#if DEBUG_XML_DETAIL
	if (gDat.args.en_xml) {
		cout << "-------------output XML string------------------" << endl;
		cout << str2SC << endl;
		cout << "----------------------------------------------------------" << endl;
	}
//#endif
	free(xmlbuff);
	xmlFreeDoc(doc);
}
bool MessageMgr::isMsgHeadAvail(string &xmlStr, s32 &idx) {
	MsgHead head;
	if ((chkXmlHead(xmlStr.c_str(), "Request", head))
	  ||(chkXmlHead(xmlStr.c_str(), "Response", head))){
//		DBG("Name:%s\n", head.str.c_str());
//		DBG("Code:%s\n", head.id.c_str());
		if (isMsgHeadTypeAvail(head, idx)) { // 不是奇数，不是返回的id！
			return true;
		}
	}
	cout << "\t** wrong head ! \n";
	return false;
}

int MessageMgr::makeResponseStr(u32 msgIdx, string &xmlStr, string &str2SC) {
	MessagePair * pMsg;
	switch (msgIdx) {
	case PK_IDX_401_GET_DATA:
		pMsg = new MsgGetDatRtnDat(msgIdx, PK_IDX_402_GET_DATA_ACK);
		break;
	case PK_IDX_403_GET_HISDATA:
		pMsg = new MsgGetHisDatRtnDat(msgIdx, PK_IDX_404_GET_HISDATA_ACK);
		break;
	case PK_IDX_1001_SET_POINT:
		pMsg = new MsgSetPointRtnDat(msgIdx, PK_IDX_1002_SET_POINT_ACK);
		break;
	case PK_IDX_1301_TIME_CHECK:
		pMsg = new MsgTimeCheckRtnDat(msgIdx, PK_IDX_1302_TIME_CHECK_ACK);
		break;
	case PK_IDX_1501_GET_LOGININFO:
		pMsg = new MsgGetFsuLoginInfoRtnDat(msgIdx,
				PK_IDX_1502_GET_LOGININFO_ACK);
		break;
	case PK_IDX_1503_SET_LOGININFO:
		pMsg = new MsgSetFsuLoginInfoRtnDat(msgIdx,
				PK_IDX_1504_SET_LOGININFO_ACK);
		break;
	case PK_IDX_1601_GET_FTP:
		pMsg = new MsgGetFtpRtnDat(msgIdx, PK_IDX_1602_GET_FTP_ACK);
		break;
	case PK_IDX_1603_SET_FTP:
		pMsg = new MsgSetFtpRtnDat(msgIdx, PK_IDX_1604_SET_FTP_ACK);
		break;
	case PK_IDX_1701_GET_FSUINFO:
		pMsg = new MsgGetFsuInfoRtnDat(msgIdx, PK_IDX_1702_GET_FSUINFO_ACK);
		break;
	case PK_IDX_1801_SET_FSUREBOOT:
		pMsg = new MsgSetFsuRebootRtnDat(msgIdx, PK_IDX_1802_SET_FSUREBOOT_ACK);
		break;
	case PK_IDX_1901_GET_THRESHOLD:
		pMsg = new MsgGetThresholdRtnDat(msgIdx, PK_IDX_1902_GET_THRESHOLD_ACK);
		break;
	case PK_IDX_2001_SET_THRESHOLD:
		pMsg = new MsgSetThresholdRtnDat(msgIdx, PK_IDX_2002_SET_THRESHOLD_ACK);
		break;
	default:
		return 0;
	}
	if (pMsg) {
		int rtn = pMsg->makeResponseStr(xmlStr, str2SC);
		delete pMsg;
		return rtn;
	} else
		return 0;
}
int MessageMgr::hdlScResponse(string &xmlData) {
	s32 idx = -1;
//#if DEBUG_XML_DETAIL
	if (gDat.args.en_xml) {
		cout << "-------------income XML string------------------" << endl;
		cout << xmlData << endl;
		cout << "------------------------------------------------" << endl;
	}
//#endif
	if (isMsgHeadAvail(xmlData, idx)) {
		switch (idx) {
		// income response
		case PK_IDX_102_LOGIN_ACK:
		case PK_IDX_104_LOGOUT_ACK:
		case PK_IDX_502_SEND_ALARM_ACK:
			return scProxy.parseSCResponse(idx, xmlData);
			break;
		}
	}
	return 0;
}
int MessageMgr::inComeMsgHdl(string &xmlData, string &str2SC) {
	s32 idx = -1;
//#if DEBUG_XML_DETAIL
	if (gDat.args.en_xml) {
		cout << "-------------income XML string------------------" << endl;
		cout << xmlData << endl;
		cout << "------------------------------------------------" << endl;
	}
//#endif
	//--------- xpath find test -----------
	//xmlXpathTest(xmlData);
	//-------------------------------------
	if (isMsgHeadAvail(xmlData, idx)) {
		switch (idx) {
		// income request
		case PK_IDX_401_GET_DATA:
		case PK_IDX_403_GET_HISDATA:
		case PK_IDX_1001_SET_POINT:
		case PK_IDX_1301_TIME_CHECK:
		case PK_IDX_1501_GET_LOGININFO:
		case PK_IDX_1503_SET_LOGININFO:
		case PK_IDX_1601_GET_FTP:
		case PK_IDX_1603_SET_FTP:
		case PK_IDX_1701_GET_FSUINFO:
		case PK_IDX_1801_SET_FSUREBOOT:
		case PK_IDX_1901_GET_THRESHOLD:
		case PK_IDX_2001_SET_THRESHOLD:
			// 开启线程来做，有些数据不能即时返回！！！
			int rtn = makeResponseStr(idx, xmlData, str2SC);
			// output ...

			//
			if (PK_IDX_1801_SET_FSUREBOOT == idx) {
				if (rtn == 1) {
					//send  a message of reboot;
				}
			}

			return rtn;
		}
	}
	return 0;
}

