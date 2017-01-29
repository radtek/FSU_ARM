/*
 * MsgReqFsuLoginInfoRtnDat.cpp
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */

#include "debug.h"
#include "AppConfig.h"
#include "xmlHdl.h"
#include "MsgGetFsuLoginInfoRtnDat.h"

extern GlobalDat gDat;

int MsgGetFsuLoginInfoRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "fsuId:" << fsuId << endl;
	cout << "fsuCode:" << fsuCode << endl;
#endif
	if (fsuId != gDat.cfgs[GIDX_FSUID])
		return 0;
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
	xmlNewChild(info, NULL, BAD_CAST "IPSecUser", 	BAD_CAST gDat.cfgs[GIDX_IPSECUSER].c_str());
	xmlNewChild(info, NULL, BAD_CAST "IPSecPWD", BAD_CAST gDat.cfgs[GIDX_IPSECPWD].c_str());
	xmlNewChild(info, NULL, BAD_CAST "IPSecIP", BAD_CAST gDat.cfgs[GIDX_IPSECIP].c_str());
	xmlNewChild(info, NULL, BAD_CAST "SCIP", BAD_CAST gDat.cfgs[GIDX_SCIP_LOGIN].c_str());

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
int MsgGetFsuLoginInfoRtnDat::parseXmlInfoPart(string &xmlStr) {
	xmlDocPtr pDoc = getMemDoc(xmlStr);
	if (pDoc) {
		const xmlChar *xpath = (const xmlChar *)("/Request/Info/FsuId");
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
					fsuId = string((char*)content);
					xmlFree(content);
					if (getNextContent(cur, (const xmlChar *)("FsuCode"), fsuCode)) {
						rtn = 1;
					}
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



