/*
 * B_Interface.cpp
 *
 *  Created on: 2016-5-13
 *      Author: vmuser
 */
//#include <sstream>
#include <stdio.h>
#include "B_Interface.h"
namespace BInt {
stInitGRP::stInitGRP(const string &sId, const string &gId) {
	semaId = sId;
#if DEBUG_INIT_DAT
	cout << "in stInitGRP -- id = " << sId << ", gid = " << gId << endl;
#endif
	grpId = gId;
	bGrp = false;
	grpNum = 0;
	bFixNum = false;
	if (isIdAvailable(gId, 8)) { // 数值从协议包读取
		grpId = gId;
		bGrp = true;
	} else {
		int sz = gId.size();
		if (sz != 0) {
			if (isIdAvailable(gId, sz)) {
				if (sz < 3) {
					int n = atoi(gId.c_str());
					if (n > 64) {
						cout << "out size of group number ! -- " << gId << endl;
					}
					else {
						bGrp = true;
						bFixNum = true;
						grpNum = n;
//						cout << "grpNum = " << n << endl;
					}
				}
			} else {
				if (sz == 3) {
					const char * s = gId.c_str();
					if ( (s[0] == 'n')
							&&((s[1] >= '0') && (s[1] <= '9'))
							&&((s[2] >= '0') && (s[2] <= '9'))) {
						grpId = gId;
						bGrp = true;
					} else
						cout << "Bad format of group info !" << endl;
				} else
					cout << "Bad format of group info !" << endl;
			}
		}
	}
}
bool strLenHdl(u32 type, string &in, string & out) {
	return false;
}
bool isIdAvailable(string id, u32 len) {
	if (id.size() != len)
		return false;
	const char * s = id.c_str();
	for (u32 i = 0; i < len; i++) {
		if ((s[i] < '0') || (s[i] > '9'))
			return false;
	}
	return true;
}
bool isIdAvailable_sema(string id) {
	u32 sz = id.size();
	if ((sz != 7) || (sz != 8))
		return false;
	const char * s = id.c_str();
	for (u32 i = 0; i < sz; i++) {
		if ((s[i] < '0') || (s[i] > '9'))
			return false;
	}
	return true;
}
string semaIdCalcAdd(const string & semaId, u32 i) { 		//	8位数字
	if (i == 0)
		return semaId;
	int id = atoi(semaId.c_str());
	id += i;
	char c[20];
	sprintf(c, "%d", id);
//	std::stringstream newStr;
//	newStr << i;
//	string newId = newStr.str();
	string newId(c);
	if (newId.size() == 7)
		newId = string("0") + newId;
	return newId;
}
bool isSameSemaType(const string & l, const string r) {
	if (l.size() != r.size())
		return false;
	unsigned int sz = l.size() - 3;
	const char * sl = l.c_str();
	const char * sr = r.c_str();
	for (unsigned int i = 0; i < sz; ++i) {
		if (*(sl + i) != *(sr + i))
			return false;
	}
	return true;
}
//void fmtSemaId(string & id) {
//	if (id.size() == 7)
//		id = "0" + id;
//}
string & fmtSemaId1(string & id) {
	if (id.size() == 7)
		id = "0" + id;
	return id;
}
bool chkFmtSemaId(string & id) {
	u32 sz = id.size();
	if ((sz != 7) && (sz != 8))
		return false;
	if (id.size() == 7)
		id = "0" + id;
	const char * s = id.c_str();
	if ((s[0] == 0) && (s[1] == 0))
		return false;
	return true;
}
void getSemaIdx2Str(const string & semaId, string & out) {
	unsigned int sz = semaId.size();
	const char * s = semaId.c_str();
	char c[3] = {0};
	c[0] = *(s + sz - 2);
	c[1] = *(s + sz - 1);
	out = string(c);
}
void getDevTypeStr(const string & devId, string & out) { // devId必须是7位的数字
	if (!isIdAvailable(devId, 7))
		out = string();
	else {
		char c3[3] = {0};
		const char * s = devId.c_str();
		c3[0] = *s;
		c3[1] = *(s+1);
		out = string(c3);
	}
}
void getDevTypeStr_fullId(const string & devId, string & out) { // devId必须是14位的数字
	if (!isIdAvailable(devId, 14))
		out = string();
	else {
		char c3[3] = {0};
		const char * s = devId.c_str();
		c3[0] = *(s + 7);
		c3[1] = *(s + 8);
		out = string(c3);
	}
}
u32 getDevType(const string & devId) { // devId必须是7位的数字
	if (!isIdAvailable(devId, 7))
		return 0;
	const char * s = devId.c_str();
	char h,l;
	h = *s - '0';
	l = *(s + 1) - '0';
	return h*10 + l;

}
} // end of name space

