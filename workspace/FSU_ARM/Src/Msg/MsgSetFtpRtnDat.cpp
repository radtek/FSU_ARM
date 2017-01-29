/*
 * MsgSetFtpRtnDat.cpp
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */

#include "debug.h"
#include "xmlHdl.h"
#include "MsgSetFtpRtnDat.h"
#include "AppConfig.h"

extern GlobalDat gDat;

int MsgSetFtpRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "FsuId:" << fsuId << endl;
	cout << "FsuCode:" << fsuCode << endl;
	cout << "UserName:" << UserName << endl;
	cout << "Password:" << Password << endl;
#endif
	if (fsuId != gDat.cfgs[GIDX_FSUID])
		return 0;

	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
	string result = string(bSuccess ? "1" : "0");
	// set FTP user name and password

	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST result.c_str());

	wrtXmlToStr(doc, str2SC);

	return 1;
}
int MsgSetFtpRtnDat::parseXmlInfoPart(string &xmlStr) {
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
					if ((getNextContent(cur, (const xmlChar *)("FsuCode"), fsuCode))
					  &&(getNextContent(cur, (const xmlChar *)("UserName"), UserName))
					  &&(getNextContent(cur, (const xmlChar *)("Password"), Password))) {
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



