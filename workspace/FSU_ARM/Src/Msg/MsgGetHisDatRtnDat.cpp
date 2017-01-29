/*
 * MsgGetHisDatRtnDat.cpp
 *
 *  Created on: 2016-3-27
 *      Author: lcz
 */
#include "debug.h"
#include "xmlHdl.h"
#include "MsgGetHisDatRtnDat.h"
#include "AppConfig.h"
#include "Device.h"

extern GlobalDat gDat;
extern DeviceManager DevMgr;

int MsgGetHisDatRtnDat::makeOutXml(bool bSuccess, string &str2SC) {
#if _SDEBUG_DETAIL_
	for (const auto & dev: m_req.devs) {
		cout << "devId:" << dev.devId << "\tCode:" << dev.code << endl;
		for(const auto & id : dev.ids)
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
	if (checkAndChangeDateFormat(startTime) && checkAndChangeDateFormat(endTime)) {
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
						sIds.push_back(id);
				}
				addTheDeviceDat(dev.devId, sIds, devsNd);
			}
		}
	} else {
		xmlNewChild(info, NULL, BAD_CAST "Result", BAD_CAST "0");
	}

	wrtXmlToStr(doc, str2SC);
	cout << "str2SC size :" << str2SC.size() << endl;
	m_req.clearDat();

	return 1;
}
void MsgGetHisDatRtnDat::addTheDeviceDat(string devId, vector<string> sIds, xmlNodePtr & pDevsNd) {
	xmlNodePtr pDevNd = NULL;
	vector<stSemaphore> vecDat;
	DevMgr.getTheDeviceSemaphoes_history(	devId, sIds, vecDat, startTime, endTime);
//	for (auto &i : vecDat) {
//		cout << "itm:" << i.id << i.measuredVal << endl;
//	}

	pDevNd = xmlNewChild(pDevsNd, NULL, BAD_CAST "Device", BAD_CAST NULL);
	xmlNewProp(pDevNd, BAD_CAST "Id", BAD_CAST devId.c_str());
	xmlNewProp(pDevNd, BAD_CAST "Code", BAD_CAST devId.c_str());
	for (const auto & sema : vecDat) {
//		cout << sema.type << "|" << sema.id << "|" << sema.measuredVal << "|" << sema.setupVal << "|" << sema.status << "|" << sema.recordTime << endl;
		xmlNodePtr pSemavNd = NULL;
		pSemavNd = xmlNewChild(pDevNd, NULL, BAD_CAST "TSemaphore", BAD_CAST NULL);
		xmlNewProp(pSemavNd, BAD_CAST "Type", 		BAD_CAST sema.type.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Id", 		BAD_CAST sema.id.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "MeasuredVal",BAD_CAST sema.measuredVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "SetupVal", 	BAD_CAST sema.setupVal.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "Status", 	BAD_CAST sema.status.c_str());
		xmlNewProp(pSemavNd, BAD_CAST "RecordTime",	BAD_CAST sema.recordTime.c_str());
	}
}
int MsgGetHisDatRtnDat::parseXmlInfoPart(string &xmlStr) {
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
//					if (getNextContent(cur, (const xmlChar *)("FsuCode"), fsuCode))
//						cout << fsuCode << endl;
//					if (getNextContent(cur, (const xmlChar *)("StartTime"), startTime))
//						cout << startTime << endl;
//					if (getNextContent(cur, (const xmlChar *)("EndTime"), endTime))
//						cout << endTime << endl;
					if ((getNextContent(cur, (const xmlChar *)("FsuCode"), fsuCode))
					  &&(getNextContent(cur, (const xmlChar *)("StartTime"), startTime))
					  &&(getNextContent(cur, (const xmlChar *)("EndTime"), endTime))) {
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

