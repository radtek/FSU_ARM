/*
 * MsgGetDatRtnDat.cpp
 *
 *  Created on: 2016-3-25
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "AppConfig.h"
#include "MsgGetDatRtnDat.h"
#include "Device.h"

extern GlobalDat gDat;
extern DeviceManager DevMgr;

int MsgGetDatRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	for (const auto & dev : m_req.devs) {
		cout << "devId:" << dev.devId << ",\tCode:" << dev.code << endl;
		for (const auto & id : dev.ids)
			cout << "\tId:" << id << endl;
	}
#endif
	if (fsuId != gDat.cfgs[GIDX_FSUID])
		return 0;

	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr info = NULL;

	makeOutXmlHead(doc, root, info);
	xmlNewChild(info, NULL, BAD_CAST "FsuId", BAD_CAST gDat.cfgs[GIDX_FSUID].c_str());
	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST gDat.cfgs[GIDX_FSUCODE].c_str());
	xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST "1");
	xmlNodePtr values = xmlNewChild(info, NULL, BAD_CAST "Values", BAD_CAST NULL);
	xmlNodePtr devsNd = xmlNewChild(values, NULL, BAD_CAST "DeviceList", BAD_CAST NULL);

	if (m_req.all) { // all devices and all semaphores.
//		vector<string> dIds;
//		DevMgr.getAllDevices(dIds);
		for (const auto & dev : gDat.vecDev) {
			vector<string> sIds;
			DevMgr.getTheDeviceAllIds(dev.Id, sIds);
			addTheDeviceDat(dev.Id, sIds, devsNd);
		}
	} else {
		for (const auto & dev : m_req.devs) {
			vector<string> sIds;
			if (dev.all) { // all semaphores of THIS device.
				DevMgr.getTheDeviceAllIds(dev.devId, sIds);
			} else {
				for (const auto & id : dev.ids) {
					string s = getSemaphoreShortId(id);
					if (s.size() == 7)
						s = "0" + s;
					sIds.push_back(s);
				}
			}
			addTheDeviceDat(dev.devId, sIds, devsNd);
		}
	}
	wrtXmlToStr(doc, str2SC);
	m_req.clearDat();

	return 1;
}
void MsgGetDatRtnDat::addTheDeviceDat(string devId, vector<string> sIds, xmlNodePtr & pDevsNd) {
	xmlNodePtr pDevNd = NULL;
	vector<stSemaphore_l> vecDat;

	for (const auto & id : sIds)
		vecDat.push_back(stSemaphore_l(id));

	DevMgr.getTheDeviceSemaphoes_current(devId, vecDat);

	pDevNd = xmlNewChild(pDevsNd, NULL, BAD_CAST "Device", BAD_CAST NULL);
	xmlNewProp(pDevNd, BAD_CAST "Id", BAD_CAST devId.c_str());
	xmlNewProp(pDevNd, BAD_CAST "Code", BAD_CAST devId.c_str());
	for (const auto & sema : vecDat) {
		string sId = sema.id;
		if (sema.id.size() == 8)
			sId = gDat.semaIdPreString + sId;

//		cout << "sId=" << sId << endl;
		xmlNodePtr pSemavNd = NULL;
		pSemavNd = xmlNewChild(pDevNd, NULL, BAD_CAST "TSemaphore", BAD_CAST NULL);
		xmlNewProp(pSemavNd, BAD_CAST "Type", 		BAD_CAST sema.type.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Id", 		BAD_CAST sId.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "MeasuredVal",BAD_CAST sema.measuredVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "SetupVal", 	BAD_CAST sema.setupVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Status", 	BAD_CAST sema.status.c_str());
	}
}
int MsgGetDatRtnDat::parseXmlInfoPart(string &xmlStr) {
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
						xpath = (const xmlChar *) ("//Device");
						xmlXPathObjectPtr _rst;
						_rst = getNodeset(pDoc, xpath);
						if (_rst) {
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

