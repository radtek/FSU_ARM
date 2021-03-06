/*
 * MsgSetPointRtnDat.cpp
 *
 *  Created on: 2016-3-29
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "MsgSetPointRtnDat.h"
#include "AppConfig.h"
#include "Device.h"

extern GlobalDat gDat;
extern DeviceManager DevMgr;

int MsgSetPointRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	cout << "FsuId:" << FsuId << endl;
	cout << "FsuCode:" << FsuCode << endl;
	for (const auto & dev: m_req.devs) {
		cout << "devId:" << dev.devId << endl;
		cout << "Code:" << dev.code << endl;
		for(const auto & sem : dev.sems)
			cout << "\tId:" << sem.id
			<< "\tType:" << sem.type
			<< "\tMeasureVal:"<< sem.measuredVal
			<< "\tSetupVal:" << sem.setupVal
			<< "\tStatus:" << sem.status
			<< endl;
	}
#endif
	if (FsuId != gDat.cfgs[GIDX_FSUID])
		return 0;
	if (!bSuccess)
		return 0;

	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;

	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());

	vector <setPointReponseOfDevice> devResult;
	for (const auto & d : m_req.devs) {
		cout << "devId=" << d.devId << endl;
		for (const auto & s : d.sems)
			cout << "\tSemId=" << s.id << ", val=" << s.setupVal << endl;
	}
	DevMgr.setPoint(m_req, devResult);
#if DEBUG
	cout << "### size at setPoint() = " << m_req.devs.size() << endl;
#endif

	bool rslt = true;
	for (const auto & dev : devResult) {
		if (dev.failedIds.size() > 0) {
			rslt = false;
			break;
		}
	}
	const char * result = rslt ? "1" : "0";
	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST result);
	xmlNodePtr pDevListNd = xmlNewChild(info, NULL, BAD_CAST "DeviceList", BAD_CAST NULL);
	xmlNodePtr pDevNd = NULL;
	for (const auto & dev : devResult) {
		pDevNd = xmlNewChild(pDevListNd, NULL, BAD_CAST "Device", BAD_CAST NULL);
		xmlNewProp(pDevNd, BAD_CAST "Id", BAD_CAST dev.devId.c_str());
		xmlNewProp(pDevNd, BAD_CAST "Code", BAD_CAST dev.devId.c_str());
		xmlNodePtr successListNd
			= xmlNewChild(pDevNd, NULL, BAD_CAST "SuccessList", BAD_CAST NULL);
		for (const auto & sid : dev.successIds)
			xmlNewChild(successListNd, NULL, BAD_CAST "Id", BAD_CAST sid.c_str());
		xmlNodePtr failedListNd
			= xmlNewChild(pDevNd, NULL, BAD_CAST "FailList", BAD_CAST NULL);
		for (const auto & sid : dev.failedIds)
			xmlNewChild(failedListNd, NULL, BAD_CAST "Id", BAD_CAST sid.c_str());
	}

	wrtXmlToStr(doc, str2SC);
	m_req.clearDat();

	return 1;
}
int MsgSetPointRtnDat::parseXmlInfoPart(string &xmlStr) {
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
					FsuId = string((char*)content);
					xmlFree(content);
					if (getNextContent(cur, (const xmlChar *)("FsuCode"), FsuCode)) {
						xpath = (const xmlChar *)("//Device");
						xmlXPathObjectPtr _rst;
						_rst = getNodeset(pDoc, xpath);
						if(_rst) {
							m_req.setDat(_rst);
							rtn = 1;
							xmlXPathFreeObject(_rst);
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




