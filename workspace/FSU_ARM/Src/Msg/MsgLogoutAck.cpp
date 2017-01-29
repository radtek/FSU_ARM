/*
 * MsgLogoutAck.cpp
 *
 *  Created on: 2016-4-2
 *      Author: lcz
 */
#include "debug.h"
#include "AppConfig.h"
#include "xmlHdl.h"
#include "MsgLogoutAck.h"
#include "LoginLogout.h"

extern GlobalDat gDat;
int MsgLogoutAck::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
#endif
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", 	BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	wrtXmlToStr(doc, str2SC);
	return 1;
}
extern LoginHdl loginHdl;
int MsgLogoutAck::parseXmlInfoPart(string &xmlStr) {
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
					if (r == 1)
//						gDat.bLogin = false;
						loginHdl.setLoginState(false);
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


