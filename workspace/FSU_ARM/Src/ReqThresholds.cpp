/*
 * ReqThreshold.cpp
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */
#include "define.h"
#include "ReqThresholds.h"

void ReqThresholds::setDat(xmlXPathObjectPtr rst) {

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
			reqDevThreholds rdt;
			rdt.devId = (char *)id;
			rdt.code = (char *)code;

			cur = cur->children;

			while (cur != NULL) {
				if (!xmlStrcmp(cur->name, (const xmlChar *)"TThreshold")) {
					xmlAttr * as = cur->properties;
					if (as) {
						stThreshold thr;
						if (!xmlStrcmp(as->name, (const xmlChar *)"Type")) {
//							if (!getEnumType((const char *)(as->children->content), thr.type))
//								continue;
							thr.type = string(const_cast<const char *>(reinterpret_cast<char*>(as->children->content)));
							as = as->next;
							if (as) {
								if (!xmlStrcmp(as->name, (const xmlChar *)"Id")) {
									if (strlen((const char*)as->children->content) == 10) {
//										for (int i = 0; i < 10; ++i)
//											thr.id[i] = as->children->content[i];
										thr.id = string((char *)as->children->content);
										as = as->next;
										if (as) {
											if (!xmlStrcmp(as->name, (const xmlChar *)"Threshold")) {
//												thr.Threshold = atof((const char *)as->children->content);
												thr.Threshold = string((char *)as->children->content);
												as = as->next;
												if (as) {
													if (!xmlStrcmp(as->name, (const xmlChar *)"AbsoluteVal")) {
//														thr.AbsoluteVal = atof((const char *)as->children->content);
														thr.AbsoluteVal = string((char *)as->children->content);
														as = as->next;
														if (!xmlStrcmp(as->name, (const xmlChar *)"RelativeVal")) {
//															thr.RelativeVal = atof((const char *)as->children->content);
															thr.RelativeVal = string((char *)as->children->content);
															as = as->next;
															if (as) {
																if (!xmlStrcmp(as->name, (const xmlChar *)"Status")) {
//																	thr.status = getEnumState((const char *)as->children->content);
																	thr.status = string((char *)as->children->content);
																	rdt.thrs.push_back(thr);
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
						}
					}
				}
				cur = cur->next;
			}
			devs.push_back(rdt);
		}
    }
}
void ReqThresholds::clearDat() {
//	for (auto & dev : devs)
//		dev.thrs.clear();
//	devs.clear();
	ClearVector(devs);
}




