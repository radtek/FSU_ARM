/*
 * MsgTimeCheckRtnDat.cpp
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "define.h"
#include "MsgTimeCheckRtnDat.h"


int MsgTimeCheckRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "Year:" << Year << endl;
	cout << "Month:" << Month << endl;
	cout << "Day:" << Day << endl;
	cout << "Hour:" << Hour << endl;
	cout << "Minute:" << Minute << endl;
	cout << "Second:" << Second << endl;
#endif
	// set Time .
	bool rslt = false;
	if (bSuccess) {
		stDateTimeStr tm;
		tm.year = Year;
		tm.mon = Month;
		tm.day = Day;
		tm.hour = Hour;
		tm.min = Minute;
		tm.sec = Second;
		rslt =  setSysTime(tm);			// ------------------- set here !
	}
	string result =  string(rslt ? "1" : "0");

	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;
	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST result.c_str());
	wrtXmlToStr(doc, str2SC);

	return 1;
}
int MsgTimeCheckRtnDat::parseXmlInfoPart(string &xmlStr) {
	xmlDocPtr pDoc = getMemDoc(xmlStr);
	if (pDoc) {
		const xmlChar *xpath = (const xmlChar *)("/Request/Info/Time");
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
					cur = cur->children;
					BLANK(cur);
					if (cur && !xmlStrcmp(cur->name, (const unsigned char *)"Years")) {
						if (cur->children) {
							const xmlChar * _content = cur->children->content;
							if (_content && (strlen((const char*)_content)!=0)) {
								Year = string((char*)_content);
								if ((getNextContent(cur, (const xmlChar *)("Month"), Month))
								  &&(getNextContent(cur, (const xmlChar *)("Day"), Day))
								  &&(getNextContent(cur, (const xmlChar *)("Hour"), Hour))
								  &&(getNextContent(cur, (const xmlChar *)("Minute"), Minute))
								  &&(getNextContent(cur, (const xmlChar *)("Second"), Second))) {
									rtn = 1;
								}
							}
						}
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




