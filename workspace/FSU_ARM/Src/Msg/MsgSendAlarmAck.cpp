/*
 * MsgSendAlarmAck.cpp
 *
 *  Created on: 2016-4-2
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "MsgSendAlarmAck.h"
#include "B_Interface.h"
#include "AlarmHdl.h"
#include "DB.h"

using namespace BInt;
extern DBHdl DB;

int MsgSendAlarmAck::makeOutXml(bool bSuccess, string &str2SC) {
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;

	DB.getUnconfirmedAlm();
#if 1
	cout << "\t * getUncofirmedAlm * \n";
	for (const auto & alm: DB.vecAlm) {
		cout << "serialNo:\t" << alm.serialNo << endl;
		cout << "id:\t" << alm.id << endl;
		cout << "fsuId:\t" << alm.fsuId << endl;
		cout << "fsuCode:\t" << alm.fsuCode << endl;
		cout << "DeviceID:\t" << alm.deviceId << endl;
		cout << "DeviceCode:\t" << alm.deviceCode << endl;
		cout << "AlarmTime:\t" << alm.alarmTime << endl;
		cout << "alarmLevel:\t" << alm.alarmLevel << endl;
		cout << "AlarmFlag:\t" << alm.alarmFlag << endl;
		cout << "AlarmDesc:\t" << alm.alarmDesc << endl;
	}
#endif
	if (DB.vecAlm.size() == 0)
		return 0;

	makeOutXmlHead(doc, root, info);
	xmlNodePtr v = xmlNewChild(info, NULL, BAD_CAST "Values", BAD_CAST NULL);
	xmlNodePtr almList = xmlNewChild(v, NULL, BAD_CAST "TAlarmList", BAD_CAST NULL);
	xmlNodePtr almNd = NULL;
	for (const auto & alm : DB.vecAlm) {
		almNd = xmlNewChild(almList, NULL, BAD_CAST "TAlarm", BAD_CAST NULL);
		xmlNewChild(almNd, NULL, BAD_CAST "SerialNo",	BAD_CAST alm.serialNo.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "Id", 		BAD_CAST alm.id.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "FsuId",		BAD_CAST alm.fsuId.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "FsuCode",	BAD_CAST alm.fsuCode.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "DeviceId",	BAD_CAST alm.deviceId.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "DeviceCode",	BAD_CAST alm.deviceCode.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "AlarmTime",	BAD_CAST alm.alarmTime.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "AlarmLevel",	BAD_CAST alm.alarmLevel.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "AlarmFlag",	BAD_CAST alm.alarmFlag.c_str());
		xmlNewChild(almNd, NULL, BAD_CAST "AlarmDesc",	BAD_CAST alm.alarmDesc.c_str());
	}

	wrtXmlToStr(doc, str2SC);

	return 1;
}
int MsgSendAlarmAck::parseXmlInfoPart(string &xmlStr) {
	xmlDocPtr pDoc = getMemDoc(xmlStr);
	if (pDoc) {
		const xmlChar *xpath = (const xmlChar *)("/Response/Info/Result");
		xmlXPathObjectPtr rst;
		rst = getNodeset(pDoc, xpath);
		int rtn = 0;
		if (rst){
			xmlNodeSetPtr nodeset = rst->nodesetval;
			xmlNodePtr cur;
			if (nodeset->nodeNr > 0) {
				cur = nodeset->nodeTab[0];
//				xmlXPathFreeObject(rst);
				if (cur) {
					xmlChar * content = xmlNodeGetContent(cur);
					string s = string((char*)content);
					xmlFree(content);
					char * end;
					char r = static_cast<char>(strtol(s.c_str(), &end, 10));
					if (r == 1)	//{// report to SC done.
						rtn = 1;
//						DB.reportBack(true);
//					} else {
//						DB.reportBack(false);
//					}
				}
			}
			xmlXPathFreeObject(rst);
		}
		xmlFreeDoc(pDoc);
//		xmlCleanupParser();
//		xmlMemoryDump();
		return rtn;
	}
	return 0;
}




