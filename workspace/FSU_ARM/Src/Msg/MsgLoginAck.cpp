/*
 * MsgLoginAck.cpp
 *
 *  Created on: 2016-4-2
 *      Author: lcz
 */
#include "debug.h"
#include "AppConfig.h"
#include "xmlHdl.h"
#include "MsgLoginAck.h"
#include "LoginLogout.h"

extern GlobalDat gDat;

int MsgLoginAck::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "FsuId:" << gDat.cfgs[GIDX_FSUID] << endl;
	cout << "FsuCode:" << gDat.cfgs[GIDX_FSUCODE] << endl;
//	cout << "FsuIP:" << gDat.cfgs[GIDX_FSUIP] << endl;
	cout << "FsuIP:" << gDat.myIp << endl;
//	for (const auto & dev: gDat.vecDev) {
//		cout << "Id:" << dev.Id << ",\tCode:" << dev.Code << endl;
//	}
#endif
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "UserName", BAD_CAST gDat.cfgs[GIDX_USERNAME].c_str());
	xmlNewChild(info, NULL, BAD_CAST "PaSCword", BAD_CAST gDat.cfgs[GIDX_PASSWORD].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuId", 	BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
//	xmlNewChild(info, NULL, BAD_CAST "FsuIP", 	BAD_CAST gDat.cfgs[GIDX_FSUIP].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuIP", 	BAD_CAST gDat.myIp.c_str());
	xmlNodePtr devs = xmlNewChild(info, NULL, BAD_CAST "DeviceList", BAD_CAST NULL);
	xmlNodePtr pdev = NULL;
	for (const auto & dev: gDat.vecDev) {
		pdev = xmlNewChild(devs, NULL, BAD_CAST "Device", BAD_CAST NULL);
		xmlNewProp(pdev, BAD_CAST "Id",   BAD_CAST dev.Id.c_str());
		xmlNewProp(pdev, BAD_CAST "Code", BAD_CAST dev.Code.c_str());
	}
	wrtXmlToStr(doc, str2SC);
	return 1;
}
extern LoginHdl loginHdl;
int MsgLoginAck::parseXmlInfoPart(string &xmlStr) {
	xmlDocPtr pDoc = getMemDoc(xmlStr);
	if (pDoc) {
		const xmlChar *xpath = (const xmlChar *)("/Response/Info/RightLevel");
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
					gDat.RightLevel = static_cast<EnumRightMode>(r);
//					gDat.bLogin = (gDat.RightLevel != BInt::EnumRightMode::INVALID);
					loginHdl.setLoginState((gDat.RightLevel != BInt::EnumRightMode::INVALID));
					rtn = 1;
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




