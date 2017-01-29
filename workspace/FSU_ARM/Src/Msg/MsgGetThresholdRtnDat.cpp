/*
 * MsgGetThresholdRtnDat.cpp
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "MsgGetThresholdRtnDat.h"
#include "AppConfig.h"
#include "Device.h"

extern GlobalDat gDat;
extern DeviceManager DevMgr;

int MsgGetThresholdRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	for (const auto & dev: m_req.devs) {
		cout << "devId:" << dev.devId << ",\tCode:" << dev.code << endl;
		for(const auto & t : dev.ids)
			cout << "\tId:" << t << endl;
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
		vector<string> dIds;
		DevMgr.getAllDevices(dIds);
		for (const auto & devId : dIds) {
			vector<string> sIds;
			DevMgr.getTheDeviceAllIds(devId, sIds);
			addTheDeviceDat(devId, sIds, devsNd);
		}
	} else {
		for (const auto & dev : m_req.devs) {
			vector<string> sIds;
			if (dev.all) { // all semaphores of THIS device.
				DevMgr.getTheDeviceAllIds(dev.devId, sIds);
			} else {
				for (const auto & id : dev.ids)
					sIds.push_back(getSemaphoreShortId(id));
			}
			addTheDeviceDat(dev.devId, sIds, devsNd);
		}
	}
	wrtXmlToStr(doc, str2SC);
	m_req.clearDat();

	return 1;
}
void MsgGetThresholdRtnDat::addTheDeviceDat(string devId, vector<string> sIds, xmlNodePtr & pDevsNd) {
	xmlNodePtr pDevNd = NULL;
	vector<stThreshold> vecDat;

	for (const auto & id : sIds) {
		stThreshold sema;
		sema.id = id;
//		if (sema.id.size() == 8)
//			sema.id = gDat.semaIdPreString + sema.id;
		u32 type = getSemaphoreType(sema.id);
		char ch[2] = {0};
		ch[0] = type + '0';
		sema.type = string(ch);
		if ((type == T_SEMA_AI) || (type == T_SEMA_DI)) {
			vecDat.push_back(sema);
//			cout << "^^^^^ threshold id:" << sema.id << ", type = " << type << endl;
		}
	}
	DevMgr.getTheDeviceThreshold(devId, vecDat);

	pDevNd = xmlNewChild(pDevsNd, NULL, BAD_CAST "Device", BAD_CAST NULL);
	xmlNewProp(pDevNd, BAD_CAST "Id", BAD_CAST devId.c_str());
	xmlNewProp(pDevNd, BAD_CAST "Code", BAD_CAST devId.c_str());
	for (const auto & sema : vecDat) {
//		cout << "^^^^^ type = " << sema.type << endl;
		string sId = sema.id;
		if (sema.id.size() == 8)
			sId = gDat.semaIdPreString + sId;
		if (getSemaphoreIdx2_fullId(sId) > 1)	// 组数据门限只有一个！
			continue;
		xmlNodePtr pSemavNd = NULL;
		pSemavNd = xmlNewChild(pDevNd, NULL, BAD_CAST "TThreshold", BAD_CAST NULL);
		xmlNewProp(pSemavNd, BAD_CAST "Type", 		BAD_CAST sema.type.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Id", 		BAD_CAST sId.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Threshold",	BAD_CAST sema.Threshold.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "AbsoluteVal",BAD_CAST sema.AbsoluteVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "RelativeVal",BAD_CAST sema.RelativeVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Status", 	BAD_CAST sema.status.c_str());
	}
}
int MsgGetThresholdRtnDat::parseXmlInfoPart(string &xmlStr) {
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






