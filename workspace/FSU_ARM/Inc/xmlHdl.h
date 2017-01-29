#ifndef _XML_HDL_H_
#define _XML_HDL_H_

#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "define.h"
#include "Message.h"


//void makeXmlFile(const char *filename);
//void parseXmlFile(const char * filename);
//void parseMemXmlStr(const char * xmlStr);
void xmlParseTest();

bool chkXmlHead(PZStr s, PZStr rootStr, MsgHead & out);
xmlDocPtr getMemDoc(string xmlDat);
xmlXPathObjectPtr getNodeset(xmlDocPtr doc, const xmlChar *xpath);
bool getNextContent(xmlNodePtr &cur, const xmlChar * id, string &content);
xmlXPathObjectPtr get_nodeset(xmlDocPtr doc, const xmlChar *xpath);

void xmlXpathTest(string xmlDat);

#endif

