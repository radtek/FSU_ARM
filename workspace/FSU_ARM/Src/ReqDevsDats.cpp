/*
 * ReqDevsDats.cpp
 *
 *  Created on: 2016-3-27
 *      Author: vmuser
 */

#include "xmlHdl.h"
#include "AppConfig.h"
#include "ReqDevsDats.h"

extern GlobalDat gDat;
void ReqDevsDats::setDat(xmlXPathObjectPtr rst) {

	xmlNodeSetPtr nodeset = rst->nodesetval;
    xmlNodePtr cur;

    for (int i = 0; i < nodeset->nodeNr; i++) {
        cur = nodeset->nodeTab[i];
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
		if (!xmlStrcmp(code/*attr->name*/, (const xmlChar *)"99999999999999")) { // means all device, all Ids
			all = true;// call return all data !
			break;
		} else {
			all = false;
			bool find = false;;
			for (const auto & dev : gDat.vecDev) {
				if ((dev.Id == string((const char *)id))
						&& (dev.Code == string((const char *)code))) {
					find = true;
					break;
				}
			}
			if (find) {// if devId in devIdList
				reqDevIds rds;
				rds.devId = (char *)id;
				rds.code = (char *)code;
				rds.all = false;

				cur = cur->children;

				while (cur != NULL) {
					if (!xmlStrcmp(cur->name, (const xmlChar *)"Id")) {
						if (cur->children) {
							const xmlChar * sid;
							sid = cur->children->content;
							if (sid && (strlen((const char*)sid)!=0)) {
								if (!xmlStrcmp(sid, (const unsigned char *)"9999999999")) {
									rds.all = true;
									break;
								} else {
									string id = string((char*)sid);
									string id_;
									if (id.size() == 10) {
										char *s = (char *)sid;
										char ch[9] = {0};
										for (u32 i = 0; i < 8; ++i)
											ch[i] = s[i + 2];
										id_ = string(ch);
									}
									rds.ids.push_back(id);
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
}
void ReqDevsDats::clearDat() {
//	for (auto & dev : devs)
//		dev.ids.clear();
//	devs.clear();
	ClearVector(devs);
}

