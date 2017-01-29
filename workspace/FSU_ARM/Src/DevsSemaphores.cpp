/*
 * ReqSettings.cpp
 *
 *  Created on: 2016-3-29
 *      Author: lcz
 */
#include "define.h"
#include "DevsSemaphores.h"

Devs_Semaphores & Devs_Semaphores ::operator = (const Devs_Semaphores & r) {
	this->clearDat();
	for (const auto & d : r.devs) {
		DevSemaphores _dev;
		_dev.devId = d.devId;
		_dev.code = d.code;
		for (const auto & s : d.sems) {
			stSemaphore_l ss(s.id);
			ss.setupVal = s.setupVal;
			_dev.sems.push_back(ss);
		}
		this->devs.push_back(_dev);
	}
	return *this;
}

void Devs_Semaphores::setDat(xmlXPathObjectPtr rst) {

	xmlNodeSetPtr nodeset = rst->nodesetval;
    xmlNodePtr cur;

    for (int i = 0; i < nodeset->nodeNr; i++) {
    	//<TSemaphore Type="" Id="" MeasuredVal="" SetupVal="" status="" />
        cur = nodeset->nodeTab[i];	// every TSemaphore
		if (cur->type != XML_ELEMENT_NODE)
			continue;
		xmlAttr * attr = cur->properties;
		const xmlChar * id;
		const xmlChar * code;
		if (attr) {
			if (!xmlStrcmp(attr->name, (const xmlChar *)"Id")) {
				id = attr->children->content;
				attr = attr->next;
				if (attr) {
					if (!xmlStrcmp(attr->name, (const xmlChar *)"Code"))
						code = attr->children->content;
				}
			}
		}
		if (1) {// if devId in devIdList
			DevSemaphores rds;
			rds.devId = (char *)id;
			rds.code = (char *)code;

			cur = cur->children;

			while (cur != NULL) {
				if (!xmlStrcmp(cur->name, (const xmlChar *)"TSemaphore")) {
					xmlAttr * as = cur->properties;
					if (as) {
						stSemaphore_l sem;
						if (!xmlStrcmp(as->name, (const xmlChar *)"Type")) {
//							if (!getEnumType((const char *)(as->children->content), sem.type))
//								continue;
							sem.type = string((char *)as->children->content);
							as = as->next;
							if (as) {
								if (!xmlStrcmp(as->name, (const xmlChar *)"Id"))
//									sem.id = (const char *)(as->children->content);
									sem.id = string((char *)as->children->content);
									as = as->next;
								if (as) {
									if (!xmlStrcmp(as->name, (const xmlChar *)"MeasuredVal")) {
//										sem.measuredVal = atof((const char *)as->children->content);
										sem.measuredVal = string((char *)as->children->content);
										as = as->next;
										if (as) {
											if (!xmlStrcmp(as->name, (const xmlChar *)"SetupVal")) {
//												sem.setupVal = atof((const char *)as->children->content);
												sem.setupVal = string((char *)as->children->content);
												as = as->next;
												if (as) {
													if (!xmlStrcmp(as->name, (const xmlChar *)"Status")) {
//														sem.status = getEnumState((const char *)as->children->content);
														sem.status = string((char *)as->children->content);
														rds.sems.push_back(sem);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				cur = cur->next;
			}
			devs.push_back(rds);
		}
    }
}
void Devs_Semaphores::clearDat() {
//	for (auto & dev : devs)
//		dev.sems.clear();
//	devs.clear();
	ClearVector(devs);
}



