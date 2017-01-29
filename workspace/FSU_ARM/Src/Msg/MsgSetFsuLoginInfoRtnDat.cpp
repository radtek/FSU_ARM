/*
 * MsgSetFsuLoginInfoRtnDat.cpp
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "AppConfig.h"
#include "MsgSetFsuLoginInfoRtnDat.h"

extern GlobalDat gDat;

int MsgSetFsuLoginInfoRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "fsuId: " << fsuId << endl;
	cout << "fsuCode: " << fsuCode << endl;
	cout << "IPSecUser: " << IPSecUser << endl;
	cout << "IPSecPWD: " << IPSecPWD << endl;
	cout << "IPSecIP: " << IPSecIP << endl;
	cout << "SCIP: " << SCIP << endl;
	for (const auto & dev: m_devs)
		cout << "Id:" << dev.Id << ",\tCode:" << dev.Code << endl;
#endif
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;

	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
	const char * result = bSuccess ? "1" : "0";
	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST result);
	wrtXmlToStr(doc, str2SC);

	return 1;
}
int MsgSetFsuLoginInfoRtnDat::parseXmlInfoPart(string &xmlStr) {
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
					if ( getNextContent(cur, (const xmlChar *)("FsuCode"), fsuCode)
					  && getNextContent(cur, (const xmlChar *)("IPSecUser"), IPSecUser)
					  && getNextContent(cur, (const xmlChar *)("IPSecPWD"), IPSecPWD)
					  && getNextContent(cur, (const xmlChar *)("IPSecIP"), IPSecIP)
					  && getNextContent(cur, (const xmlChar *)("SCIP"), SCIP)) {
						cur = cur->next->next;
						if (cur && !xmlStrcmp(cur->name, (const xmlChar *)("DeviceList"))) {
							xpath = (const xmlChar *)("//Device");
							xmlXPathObjectPtr _rst;
							_rst = getNodeset(pDoc, xpath);
							if(_rst) {
								xmlNodeSetPtr nodeset = _rst->nodesetval;
							    xmlNodePtr cur;
							    for (int i = 0; i < nodeset->nodeNr; i++) {
							        cur = nodeset->nodeTab[i];
									if (cur->type != XML_ELEMENT_NODE)
										continue;
									xmlAttr * attr = cur->properties;
									//xmlChar * content = xmlNodeGetContent(cur);
									const xmlChar * id;
									const xmlChar * code;
									if (attr) {
										if (!xmlStrcmp(attr->name, (const xmlChar *)"Id")) {
											id = attr->children->content;
											if (id && (strlen((const char*)id)!=0)) {
												attr = attr->next;
												if (attr) {
													if (!xmlStrcmp(attr->name, (const xmlChar *)"Code"))
														code = attr->children->content;
													if (code && (strlen((const char*)code)!=0)) {
														dev d;
														d.Id = (char *)id;
														d.Code = (char *)code;

														m_devs.push_back(d);
													}
												}
											}
										}
									}
							    }
								rtn = 1;
								xmlXPathFreeObject(_rst);
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




