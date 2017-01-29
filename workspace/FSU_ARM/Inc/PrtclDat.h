/*
 * PrtclDat.h
 *
 *  Created on: 2016年12月6日
 *      Author: lcz
 */

#ifndef INC_PRTCLDAT_H_
#define INC_PRTCLDAT_H_

#include "define.h"
#include <vector>
#include <string>

using namespace std;
struct P_GetItm {
#if 1//DEBUG_PROTOCOL_PARSE_FILE_DESC
//	P_GetItm(string i, bool b, u32 l, s32 m, u32 ty, string desc) : id(i), bByte(b), len(l), mul(m), type(ty), _desc(desc) {}
	P_GetItm(string i, u32 b, u32 B, s32 m, u32 ty, string desc, bool s)
		: id(i), bLen(b), BLen(B), mul(m), type(ty), _desc(desc), bSign(s) {}
#else
//	P_GetItm(string i, bool b, u32 l, s32 m, u32 ty)	: id(i), bByte(b), len(l), mul(m), type(ty) {}
	P_GetItm(string i, bool b, u32 B, s32 m, u32 ty, bool s)
	: id(i), bLen(b), BLen(B), mul(m), type(ty), bSign(s) {}
#endif
	string id;
//	bool bByte; // byte or bit
	unsigned int bLen;	// bit
	unsigned int BLen;	// Byte
	int mul;
	int type;
#if 1//DEBUG_PROTOCOL_PARSE_FILE_DESC
	string _desc;
#endif
	bool bSign;
};
struct P_Get {
	string cmdInf;
	vector<P_GetItm> itms;
};
struct P_SetCtrlItm {
//	P_SetCtrlItm(string v, string i, bool b, u32 l, s32 m) : val(v), id(i), bByte(b), len(l), mul(m) {}
	P_SetCtrlItm(string v, string i, u32 b, u32 B, s32 m, bool s)
	: val(v), id(i), bLen(b), BLen(B), mul(m), bSign(s) {}
	string val;
	string id;
//	bool bByte;	// byte or bit
	unsigned int bLen;	// bit
	unsigned int BLen;	// Byte
	int mul;
	bool bSign;
};
struct P_SetCtrl {
	string cmdInf;
	vector <P_SetCtrlItm> itms;
};
struct P_SameItm {
	P_SameItm(string v, string i, string d)
			: val(v), id(i), dxx(d) {}
	string val;
	string id;
	string dxx;	// idx
};
struct P_ByteBitsItm {
	P_ByteBitsItm(string i, string bits, string b)
		:id(i), strLen(bits), bxx(b) {}
	string id;
	string strLen;
	string bxx;
};
struct P_ByteBit {
	string bxx;
	std::vector<std::pair<string , u32>> itms;
};

bool getIdx_xx(char * s, u32 & idx);

class PrtclDat {
public:
	PrtclDat();
	~PrtclDat();
	bool initProtocolParseDat(string &file);
	string devName;
	vector <P_Get>	vecGet;
	vector <P_SetCtrl>	vecScSet;
	vector <P_SetCtrl>	vecScCtrl;
	vector <P_SameItm>	vecSame;
	vector <P_ByteBitsItm> vecBitInit;
	vector <P_ByteBit> vecBit;
	u8 * L_mem = NULL;
};



#endif /* INC_PRTCLDAT_H_ */
