#include "define.h"

#include <iostream>
#include <sstream>
#include <string.h>

#include "xmlHdl.h"
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>

using namespace std;

const char * story =
		"<?xml version=\"1.0\"?>\
	<story>\
		<storyinfo>\
			<author>John Fleck</author>\
			<datewritten>June 2, 2002</datewritten>\
			<keyword>example keyword</keyword>\
		</storyinfo>\
		<body>\
			<headline>This is the headline</headline>\
			<para>This is the body text.</para>\
		</body>\
	</story>";
const char * packetTestStr =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
		<Request>\
			<PK_Type>\
				<Name>GET_DATA</Name>\
				<Code>401</Code>\
			</PK_Type>\
			<Info>\
				<FsuID/>\
				<FsuCode/>\
			</Info>\
		</Request>";

void makeXmlFile(const char *filename) {
	xmlDocPtr doc = NULL; //文件指针
	xmlNodePtr root = NULL, node = NULL, node1 = NULL; //节点指针
//	xmlNodePtr nd2 = NULL;

	// 创建一个文件，以及设置一个根节点
	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "root");
	xmlDocSetRootElement(doc, root);

	//创建一个绑定在根节点的子节点
	xmlNewChild(root, NULL, BAD_CAST "node1", BAD_CAST "long long ago"); // 1st

	//通过xmlNewProp()增加一个节点的属性
	xmlNewChild(root, NULL, BAD_CAST "nd2", BAD_CAST ""); // 2nd

	node = xmlNewChild(root, NULL, BAD_CAST "node3",
			BAD_CAST "this node has attributes and sub node"); // 3rd
	xmlNewProp(node, BAD_CAST "attr1", BAD_CAST "yes");
	xmlNewProp(node, BAD_CAST "attribute2", BAD_CAST "no");
	//
	xmlNewChild(node, NULL, BAD_CAST "id", BAD_CAST "10111011110111");
	xmlNewChild(node, NULL, BAD_CAST "id", BAD_CAST "101110011abc11");

	//
	node = xmlNewChild(root, NULL, BAD_CAST "TThreshold", NULL); // 4th
	xmlNewProp(node, BAD_CAST "Type", BAD_CAST "");
	xmlNewProp(node, BAD_CAST "Id", BAD_CAST "0123456789");
	xmlNewProp(node, BAD_CAST "Threshold", BAD_CAST "55.40");
	xmlNewProp(node, BAD_CAST "AbsoluteVal", BAD_CAST "55.40");
	xmlNewProp(node, BAD_CAST "RelativeVal", BAD_CAST "80");

	//创建节点的另一种方法
	node = xmlNewNode(NULL, BAD_CAST "node5"); // 5th
	node1 = xmlNewText(BAD_CAST "other way to create content");
	xmlAddChild(node, node1);
	xmlAddChild(root, node);

	// 转换成字符串
	xmlChar *xmlbuff;
	int buffersize;
	xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &buffersize, "UTF-8", 1);
	cout << " make xml file test :" << endl << xmlbuff << endl;
	//保存文件
	xmlSaveFileEnc(filename, doc, "UTF-8");
	free(xmlbuff);
	/*free the document */
	xmlFreeDoc(doc);
}
void makeXmlString() {
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
//	xmlNodePtr info = NULL;

	doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewNode(NULL, BAD_CAST "root");
	xmlDocSetRootElement(doc, root);
	xmlNodePtr head = xmlNewNode(NULL, BAD_CAST "PK_TYPE");
	xmlAddChild(root, head);
//	string name = "name";
//	string id = "id";
//	xmlNewChild(head, NULL, BAD_CAST "Name",BAD_CAST name.c_str());
//	xmlNewChild(head, NULL, BAD_CAST "Code",BAD_CAST id.c_str());
//	info = xmlNewChild(root, NULL, BAD_CAST "Info", BAD_CAST NULL);
//
//	xmlNewChild(info, NULL, BAD_CAST "UserName", BAD_CAST "GIDX_USERNAME");
//	xmlNewChild(info, NULL, BAD_CAST "PaSCword", BAD_CAST "GIDX_PASSWORD");
//	xmlNewChild(info, NULL, BAD_CAST "FsuId", 	BAD_CAST "GIDX_FSUID");
//	xmlNewChild(info, NULL, BAD_CAST "FsuCode", BAD_CAST "GIDX_FSUCODE");
//	xmlNewChild(info, NULL, BAD_CAST "FsuIP", 	BAD_CAST "GIDX_FSUIP");

	//	xmlNodePtr devs = xmlNewChild(info, NULL, BAD_CAST "DeviceList", BAD_CAST NULL);
//	xmlNodePtr pdev = NULL;
//	for (const auto & dev: gDat.vecDev) {
//		pdev = xmlNewChild(devs, NULL, BAD_CAST "Device", BAD_CAST NULL);
//		xmlNewProp(pdev, BAD_CAST "Id",   BAD_CAST dev.Id.c_str());
//		xmlNewProp(pdev, BAD_CAST "Code", BAD_CAST dev.Code.c_str());
//	}

	// 转换成字符串
	xmlChar *xmlbuff;
	int buffersize;
	xmlDocDumpFormatMemoryEnc(doc, &xmlbuff, &buffersize, "UTF-8", 1);
	cout << " make xml file test :" << endl << xmlbuff << endl;
	free(xmlbuff);
	/*free the document */
	xmlFreeDoc(doc);
}

void EnumXmlElement(xmlNode * a_node) {
	xmlNodePtr node = NULL;
	// 遍历节点
	for (node = a_node; node; node = node->next) {
		if (node->type == XML_ELEMENT_NODE) {
			cout << "\t<" << node->name;
			xmlAttr * attr = node->properties;
			xmlChar * content = xmlNodeGetContent(node);
			if (attr) {
				// 遍历属性列表
				while (attr) {
					printf(" %s=\"%s\"", attr->name, attr->children->content);
					attr = attr->next;
				}
			}
			if (content && (strlen((const char*) content) != 0)) {
				cout << ">" << endl;
				cout << "\t\t" << content << endl;
				cout << "\t</" << node->name << ">" << endl;
			} else {
				cout << "/>" << endl;
			}
			xmlFree(content);
			// 递归遍历子节点列表
			EnumXmlElement(node->children);
		}
	}
}
void parseXmlFile(const char * filename) {
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;

	/*
	 * this initialize the library and check potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */LIBXML_TEST_VERSION

	/*parse the file and get the DOM */
	doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL) {
		printf("error: could not parse file %s\n", filename);
		return;
	}

	/*Get the root element node */
	root = xmlDocGetRootElement(doc);
	cout << "<" << root->name << ">" << endl;
	;

	EnumXmlElement(root->children);
	cout << "</" << root->name << ">" << endl;
	/*free the document */
	xmlFreeDoc(doc);
}
void parseMemXmlStr(const char * xmlStr) {
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	// 分析内存字符串
	doc = xmlParseMemory(xmlStr, strlen(story));
	if (doc == NULL) {
		cout << "Failed to parse memory string." << endl;
		return;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		cout << "Failed to get root node.\n";
		goto FAILED;
	}
	/*Get the root element node */
	root = xmlDocGetRootElement(doc);
	cout << "<" << root->name << ">" << endl;
	;

	EnumXmlElement(root->children);
	cout << "</" << root->name << ">" << endl;

	xmlFreeDoc(doc);

	return;
	FAILED: if (doc)
		xmlFreeDoc(doc);

}
void xmlParseTest() {
	auto s = "#Make XML file ...";
	cout << s << endl;
	makeXmlFile("testXmlFile.xml");
	makeXmlString();
	cout << "#Parse XML File and print it ." << endl;
	parseXmlFile("testXmlFile.xml");
	cout << "#Parse XML string in memory and print it." << endl;
	parseMemXmlStr(story);

}
bool chkXmlHead(PZStr s, PZStr rootStr, MsgHead & out) { //Pk_Type,Name,Code?
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;
	xmlNodePtr cur = NULL;

	doc = xmlParseMemory(s, strlen(s));
//	doc = xmlReadFile(s, NULL, XML_PARSE_NOBLANKS);
	if (doc == NULL) {
		cout << "Failed to parse memory string." << endl;
		return false;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		cout << "Failed to get root node.\n";
		goto FAILED;
	}
	if (!xmlStrcmp(root->name, BAD_CAST rootStr)) { // Request | Respond
		cur = root->children;
		NODE_VERIFY(cur);
		IF_NODE_IS(cur, PK_Type) {
			cur = cur->children;
			NODE_VERIFY(cur);
			IF_NODE_IS(cur, Name) {
				xmlChar * content = xmlNodeGetContent(cur);
				out.str = (char *) content;
				xmlFree(content);
				cur = cur->next;
				NODE_VERIFY(cur);
				IF_NODE_IS(cur, Code) {
					xmlChar * _content = xmlNodeGetContent(cur);
					out.id = (char *) _content;
					xmlFree(_content);
					xmlFreeDoc(doc);
					return true;
				}
			}
		}
	}
	FAILED: xmlFreeDoc(doc);
	return false;
}
xmlXPathObjectPtr getNodeset(xmlDocPtr doc, const xmlChar *xpath) {
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	context = xmlXPathNewContext(doc);

	if (context == NULL) {
		cout << "context is NULL" << endl;
		return NULL;
	}

	result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (result == NULL) {
		cout << "xmlXPathEvalExpression return NULL" << endl;
		return NULL;
	}

	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		xmlXPathFreeObject(result);
		cout << "nodeset is empty" << endl;
		return NULL;
	}

	return result;
}
bool getNextContent(xmlNodePtr &cur, const xmlChar * id, string &content) {
//	cur = cur->next->next;
	BLANK(cur);
	cur = cur->next;
//	cout << "name = " << cur->name << endl;
	if (cur && !xmlStrcmp(cur->name, id)) {
		if (cur->children) {
			const xmlChar * _content = cur->children->content;
			if (_content && (strlen((const char*) _content) != 0)) {
				content = string((char*) _content);
//				cout << "content = " << content << endl;
				return true;
			}
		}
	}
	return false;
}
xmlDocPtr getMemDoc(string xmlDat) {
	xmlDocPtr doc = NULL;
	doc = xmlParseMemory(xmlDat.c_str(), xmlDat.size());
	if (doc == NULL)
		cout << "Failed to get memory doc !" << endl;
	return doc;
}
void xmlXpathTest(string xmlDat) {
	cout << "----------------xmlXpath test------------------------" << endl;
	xmlDocPtr ptr = getMemDoc(xmlDat);
	if (ptr == NULL) {
		cout << "xmlXpathTest error: bad XML data !" << endl;
		return;
	}
	xmlChar *xpath = BAD_CAST ("//Device"); //关键在这行
	xmlXPathObjectPtr rst = getNodeset(ptr, xpath);

	if (rst == NULL) {
		cout << "not find !" << endl;
		cout << "----------------xmlXpath test bad end-------------------"
				<< endl;
		return;
	}

	int i = 0;
	if (rst) {
		xmlNodeSetPtr nodeset = rst->nodesetval;
		xmlNodePtr cur;

		for (i = 0; i < nodeset->nodeNr; i++) {
			cur = nodeset->nodeTab[i];
			cur = cur->xmlChildrenNode;

			while (cur != NULL) {
				if (!xmlStrcmp(cur->name, (const xmlChar *) "Id")) {
					if (cur->xmlChildrenNode) {
						//					if (cur->xmlChildrenNode->type != XML_ELEMENT_NODE)
						cout << (char*) XML_GET_CONTENT(cur->xmlChildrenNode)<< endl;
					}
				}
				cur = cur->next;
			}
		}

		xmlXPathFreeObject(rst);
	}
	cout << "----------------xmlXpath test ok -----------------------" << endl;

}

