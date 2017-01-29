/*
 * MsgGetFsuInfoRtnDat.cpp
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */
#include "define.h"
#include "debug.h"
#include "xmlHdl.h"
#include "AppConfig.h"
#include "LoginLogout.h"
#include "MsgGetFsuInfoRtnDat.h"

extern GlobalDat gDat;
extern LoginHdl loginHdl;
int MsgGetFsuInfoRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "FsuId:" << fsuId << endl;
	cout << "FsuCode:" << fsuCode << endl;
#endif
	if (fsuId != gDat.cfgs[GIDX_FSUID])
		return 0;
	loginHdl.heartBreak();
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
	xmlNodePtr stat = xmlNewChild(info, NULL, BAD_CAST "TFSUStatus", BAD_CAST NULL);
	bool rslt = false;
	if (bSuccess) {
		string c, m;
		getOccRate(c, m);
		xmlNewChild(stat, NULL, BAD_CAST "CPUUsage", BAD_CAST c.c_str());
		xmlNewChild(stat, NULL, BAD_CAST "MEMUsage", BAD_CAST m.c_str());
		rslt = true;
	} else {
		xmlNewChild(stat, NULL, BAD_CAST "CPUUsage", BAD_CAST "");
		xmlNewChild(stat, NULL, BAD_CAST "MEMUsage", BAD_CAST "");
		rslt = false;
	}
	string result =  string(rslt ? "1" : "0");
	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST result.c_str());
	wrtXmlToStr(doc, str2SC);

	return 1;
}
int MsgGetFsuInfoRtnDat::parseXmlInfoPart(string &xmlStr) {
	xmlDocPtr pDoc = getMemDoc(xmlStr);
	if (pDoc) {
		const xmlChar *xpath = (const xmlChar *) ("/Request/Info/FsuId");
		xmlXPathObjectPtr rst;
		rst = getNodeset(pDoc, xpath);
		int rtn = 0;
		if (rst) {
			xmlNodeSetPtr nodeset = rst->nodesetval;
			xmlNodePtr cur;
			if (nodeset->nodeNr > 0) {
				cur = nodeset->nodeTab[0];
//				xmlXPathFreeObject(rst);
				if (cur) {
					xmlChar * content = xmlNodeGetContent(cur);
					fsuId = string((char*) content);
					xmlFree(content);
					if (getNextContent(cur, (const xmlChar *) ("FsuCode"),
							fsuCode)) {
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

