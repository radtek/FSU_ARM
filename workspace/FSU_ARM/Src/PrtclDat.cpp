/*
 * PrtclDat.cpp
 *
 *  Created on: 2016年12月6日
 *      Author: lcz
 */
#include "PrtclDat.h"
#include "B_Interface.h"
#include "define.h"
//#include "DevComm.h"
#include "AppConfig.h"

using namespace BInt;
extern GlobalDat gDat;
bool getIdx_xx(char * s, u32 & idx) {
	u32 h = *s - '0';
	u32 l = *(s + 1) - '0';
	if (((h >= 0) && (h <= 9)) && ((l >= 0) && (l <= 9))) {
		idx = h * 10 + l;
		return true;
	}
	return false;
}
PrtclDat::PrtclDat() {}
PrtclDat::~PrtclDat() {
	if (L_mem != NULL) {
		delete[] L_mem;
		L_mem = NULL;
	}
}
bool PrtclDat::initProtocolParseDat(string &file) {
	vector<vector<string> > d;
	if (!readCsv(d, file))
		return false;
	devName = file;
	struct parseDat {
		string datType;
		string cmdInf;
		string val;
		string id;
		string desc;
		string bitLen;
		string byteLen;
		string mul;
		bool sign;
	};
	u32 nowType = -1;
	u32 lastType = -1;
	const static u32 T_GET = 0;
	const static u32 T_SET = 1;
	const static u32 T_CTRL = 2;
	const static u32 T_SAME = 3;
	const static u32 T_BIT = 4;

	P_Get getCmd;
	P_SetCtrl setCmd;
	P_SetCtrl ctrlCmd;
	P_ByteBit byteBit;
	s32 brackets = 0;
	commDevSemeIdAvailItm avi;
	avi.modelName = file;
	for (u32 x = 1; x < d.size(); x++) { // 1开始，跳过表头
#if DEBUG_PROTOCOL_PARSE_FILE
//		cout << "[" << x << "] size = " << d[x].size() << " -> ";
			for (u32 i = 0; i < d[x].size(); ++i) {
				cout << d[x][i] << ",";
			}
			cout << endl;
#endif
		parseDat p;
		bool bNewSection;
		bool bNewCmd;
		bool eof = false;
		bool bAvail;
//		string strNowLen;
//		u32 nowLen;
		if ((d[x].size() == 0)
			|| ((d[x].size() == 1) && (d[x][0].size() == 0))) { // emplty line
			//eof = true;
			continue;
		} else {

			if (d[x].size() < 9) {
				cout << "line[" << x << "]:";
				for (u32 i = 0; i < d[x].size(); ++i)
					cout << "|" << d[x][i];
				cout << endl;
				cout << "file format error !" << endl;
				return false;
			}
			p.datType = d[x][0];
			p.cmdInf = d[x][1];
			p.val = d[x][2];
			p.id = d[x][3];
			if (p.id.size() == 7)
				p.id = string("0") + p.id;
			if ((p.id.size() == 8) && !isIdAvailable(p.id, 8)) {
				cout << "\n\n\t* * *wrong id (" << p.id
						<< ") in initProtocolParseDat()* * *\n\n" << endl;
				continue;
			}
			p.desc = d[x][4];
			p.bitLen = d[x][5];
			p.byteLen = d[x][6];
			if ((p.bitLen.size() > 0) && (p.byteLen.size() > 0)) {
				cout << "\n\n\n\t* * *data len wrong !!! * * *\n\n\n" << endl;
				continue;
			}
			p.mul = d[x][7];
			p.sign = d[x][8].size() > 0;
			if ((p.mul.size() == 0) || (p.mul == string("0")))
				p.mul = string("1");
			bNewSection = p.datType.size() > 0;
			bNewCmd = p.cmdInf.size() > 0;
			bAvail = (bNewCmd || p.bitLen.size() > 0) || (p.byteLen.size() > 0)
					|| (p.val.size() > 0);
			if (!bAvail)
				continue;
			if ((p.datType == "GET") || (p.datType == "Get"))
				nowType = T_GET;
			else if ((p.datType == "SET") || (p.datType == "Set"))
				nowType = T_SET;
			else if ((p.datType == "CTRL") || (p.datType == "Ctrl"))
				nowType = T_CTRL;
			else if ((p.datType == "SAME") || (p.datType == "Same"))
				nowType = T_SAME;
			else if ((p.datType == "BIT") || (p.datType == "Bit"))
				nowType = T_BIT;
			//else // 保持上次，忽略不认识的标记
		}

		if (bNewSection || eof) {
			if (lastType >= 0) {
				switch (lastType) {
				case T_GET:
					vecGet.push_back(getCmd);
					break;
				case T_SET:
					vecScSet.push_back(setCmd);
					break;
				case T_CTRL:
					vecScCtrl.push_back(ctrlCmd);
					break;
//				case T_SAME:
//					vecBit.push_back();
//					break;
				}
			}
			switch (nowType) {
			case T_BIT: {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "new Bit" << endl;
#endif
				P_ByteBitsItm itm(p.id, p.bitLen, p.val);
				vecBitInit.push_back(itm);
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "\t\tBit itm:" << p.val << "," << p.id << "," << p.bitLen << endl;
#endif
				lastType = nowType;
			}
				break;
			case T_SAME: {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "new Same" << endl;
#endif
				P_SameItm itm(p.val, p.id, p.bitLen);
				vecSame.push_back(itm);
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "\t\tSame itm:" << p.id << endl;
#endif
				lastType = nowType;
			}
				break;
			case T_GET: {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "new Get" << endl;
#endif
				if (bNewCmd) {
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\tnew get cmdInf:" << p.cmdInf << endl;
#endif
					getCmd.itms.clear();
					getCmd.cmdInf = p.cmdInf;
					const char * s = p.val.c_str();
					u32 bracketType = T_GRP_BRACKET_NOT;
					char lr = *s;
					switch (lr) {
					case '{':
						bracketType = T_GRP_BRACKET_LEFT;
						brackets++;
						break;
					case '}':
						bracketType = T_GRP_BRACKET_RIGTH;
						brackets--;
						if (brackets < 0) {
							cout << "wrong label defined, right brackets first!"
									<< endl;
							return false;
						}

						break;
					}
#if 1//DEBUG_PROTOCOL_PARSE_FILE_DESC
					getCmd.itms.push_back(P_GetItm(p.id,
							atoi(p.bitLen.c_str()),
							atoi(p.byteLen.c_str()),
							atoi(p.mul.c_str()), bracketType, p.desc,
							p.sign));
#else
					getCmd.itms.push_back(
							P_GetItm(p.id,
									atoi(p.bitLen.c_str()),
									atoi(p.byteLen.c_str()),
									 atoi(p.mul.c_str()),
									bracketType,
									p.sign));
#endif
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew get id:" << p.id << endl;
#endif
				}
				lastType = nowType;
			}
				break;
			case T_SET: {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "new Set" << endl;
#endif
				if (bNewCmd) {
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\tnew set cmdInf:" << p.cmdInf << endl;
#endif
					setCmd.itms.clear();
					setCmd.cmdInf = p.cmdInf;
					setCmd.itms.push_back(
							P_SetCtrlItm(p.val, p.id,
									atoi(p.bitLen.c_str()),
									atoi(p.byteLen.c_str()),
									atoi(p.mul.c_str()),
									p.sign));
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew set id:" << p.id << "," << p.val << endl;
#endif
				}
				lastType = nowType;
			}
				break;
			case T_CTRL: {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "new Ctrl" << endl;
#endif
				if (bNewCmd) {
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\tnew ctrl cmdInf:" << p.cmdInf << endl;
#endif
					ctrlCmd.itms.clear();
					ctrlCmd.cmdInf = p.cmdInf;
					ctrlCmd.itms.push_back(
							P_SetCtrlItm(p.val, p.id,
									atoi(p.bitLen.c_str()),
									atoi(p.byteLen.c_str()),
									atoi(p.mul.c_str()),
									p.sign));
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew ctrl id:" << p.id << "," << p.val << endl;
#endif
				}
				lastType = nowType;
			}
				break;
			}
		} else { // not new section
			if (lastType == T_BIT) {
				P_ByteBitsItm itm(p.id, p.bitLen, p.val);
				vecBitInit.push_back(itm);
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "\t\tBit itm:" << p.val << "," << p.id << "," << p.bitLen << endl;
#endif
			} else if (lastType == T_SAME) {
				P_SameItm itm(p.val, p.id, p.bitLen);
				vecSame.push_back(itm);
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "\t\tSame itm:" << p.id << endl;
#endif
			} else {
				if (nowType == T_GET) {
					if (bNewCmd) {
						vecGet.push_back(getCmd);
						getCmd.itms.clear();
#if DEBUG_PROTOCOL_PARSE_FILE
						cout << "\t\tnew get cmdInf:" << p.cmdInf << endl;
#endif
						getCmd.cmdInf = p.cmdInf;
					}
					const char * s = p.val.c_str();
					u32 bracketType = T_GRP_BRACKET_NOT;
					char lr = *s;
					switch (lr) {
					case '{':
						bracketType = T_GRP_BRACKET_LEFT;
						brackets++;
						break;
					case '}':
						bracketType = T_GRP_BRACKET_RIGTH;
						brackets--;
						break;
					}
#if 1//DEBUG_PROTOCOL_PARSE_FILE_DESC
					getCmd.itms.push_back(P_GetItm(p.id,
							atoi(p.bitLen.c_str()),
							atoi(p.byteLen.c_str()),
							atoi(p.mul.c_str()), bracketType, p.desc,
							p.sign));
#else
					getCmd.itms.push_back(P_GetItm(p.id,
							atoi(p.bitLen.c_str()),
							atoi(p.byteLen.c_str()),
							atoi(p.mul.c_str()),
									bracketType,
									p.sign));
#endif
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew get id:" << p.id;
					if (lr == '{')
					cout << "{";
					else if (lr == '}')
					cout << "}";
					cout << endl;
#endif
				} else if (nowType == T_SET) {
					if (bNewCmd) {
						vecScSet.push_back(setCmd);
						setCmd.itms.clear();
#if DEBUG_PROTOCOL_PARSE_FILE
						cout << "\t\tnew set cmdInf:" << p.cmdInf << endl;
#endif
						setCmd.cmdInf = p.cmdInf;
					}
					setCmd.itms.push_back(
							P_SetCtrlItm(p.val, p.id,
									atoi(p.bitLen.c_str()),
									atoi(p.byteLen.c_str()),
									atoi(p.mul.c_str()),
									p.sign));
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew set id:" << p.id << "," << p.val << endl;
#endif
				} else if (nowType == T_CTRL) {
					if (bNewCmd) {
						vecScCtrl.push_back(ctrlCmd);
						ctrlCmd.itms.clear();
#if DEBUG_PROTOCOL_PARSE_FILE
						cout << "\t\tnew ctrl cmdInf:" << p.cmdInf << endl;
#endif
						ctrlCmd.cmdInf = p.cmdInf;
					}
					ctrlCmd.itms.push_back(
							P_SetCtrlItm(p.val, p.id,
									atoi(p.bitLen.c_str()),
									atoi(p.byteLen.c_str()),
									atoi(p.mul.c_str()),
									p.sign));
#if DEBUG_PROTOCOL_PARSE_FILE
					cout << "\t\t\t\tnew ctrl id:" << p.id << "," << p.val << endl;
#endif
				}
			}
		}
	}
	if (lastType >= 0) {
		switch (lastType) {
		case T_GET:
			vecGet.push_back(getCmd);
			break;
		case T_SET:
			vecScSet.push_back(setCmd);
			break;
		case T_CTRL:
			vecScCtrl.push_back(ctrlCmd);
			break;
//				case T_SAME:
//					vecBit.push_back();
//					break;
		}
	}
#if DEBUG_PROTOCOL_PARSE_FILE
	cout << "cfg.parse file = " << file << ":" << endl;
	cout << "---- GET ---------------------------" << endl;
	for (auto & i : vecGet) {
		cout << "\t\tcmdInf = " << i.cmdInf << endl;
		for (auto & j : i.itms) {
			cout << "\t\t\t\t id = " << j.id << endl;
//			if (!j.id.empty() && ((j.BLen > 0) || (j.bLen > 0))) {
//				setSemaAvailable(j.id);
//			}
		}
	}
	cout << "---- SET ---------------------------" << endl;
	for (auto & i : vecScSet) {
		cout << "\t\tcmdInf = " << i.cmdInf << endl;
		for (auto & j : i.itms) {
			cout << "\t\t\t\t id = " << j.id << endl;
//			if (!j.id.empty() && ((j.BLen > 0) || (j.bLen > 0))) {
//				setSemaAvailable(j.id);
//			}
		}
	}
	cout << "---- CTRL --------------------------" << endl;
	for (auto & i : vecScCtrl) {
		cout << "\t\tcmdInf = " << i.cmdInf << endl;
		for (auto & j : i.itms) {
			cout << "\t\t\t\t id = " << j.id << endl;
//			if (!j.id.empty()) {
//				setSemaAvailable(j.id);
//			}
		}
	}
	cout << "---- SAME --------------------------" << endl;
	for (auto & i : vecSame) {
		cout << "\t\t id = " << i.id << "\t" << i.val << "\t" << i.dxx << endl;
//		if (!i.id.empty() && !i.val.empty()) {
//			setSemaAvailable(i.id);
//		}
	}
	cout << "---- BITs --------------------------" << endl;
	for (auto & i : vecBitInit) {
		cout << "\t\t id = " << i.id << "\t" << i.strLen << "\t" << i.bxx << endl;
//		if (!i.id.empty() && (i.strLen.size() > 0)) {
//			setSemaAvailable(i.id);
//		}
	}
#endif
	///////////////////////////////////////////////////////////////////////////////////
	if (vecGet.empty())
		cout << "\n\t*** No Get Item ! ***\n\n";
	for (auto & i : vecGet) {
		for (auto & j : i.itms) {
			if (!j.id.empty() && ((j.BLen > 0) || (j.bLen > 0))) {
				//setSemaAvailable(j.id);
				avi.ids.push_back(fmtSemaId1(j.id));
			}
		}
	}
	for (auto & i : vecScSet) {
		for (auto & j : i.itms) {
			if (!j.id.empty() && ((j.BLen > 0) || (j.bLen > 0))) {
				//setSemaAvailable(j.id);
				avi.ids.push_back(fmtSemaId1(j.id));
			}
		}
	}
	for (auto & i : vecScCtrl) {
		for (auto & j : i.itms) {
			if (!j.id.empty()) {
				//setSemaAvailable(j.id);
				avi.ids.push_back(fmtSemaId1(j.id));
			}
		}
	}
	for (auto & i : vecSame) {
		if (!i.id.empty() && !i.val.empty()) {
			//setSemaAvailable(i.id);
			avi.ids.push_back(fmtSemaId1(i.id));
		}
	}
	for (auto & i : vecBitInit) {
		if (!i.id.empty() && (i.strLen.size() > 0)) {
			//setSemaAvailable(i.id);
			avi.ids.push_back(fmtSemaId1(i.id));
		}
	}
	///////////////////////////////////////////////////////////////////////////////////
	if (brackets != 0) {
		cout << "wrong label defined, miss right brackets !" << endl;
		return false;
	}
	u32 maxIdx = 0;
	for (auto & i : vecGet) {
		for (auto & j : i.itms) {
			if (j.id.size() == 3) {
				const char * s = j.id.c_str();
				if ((*s == 'L') || (*s == 'l')) {
					u32 idx = 0;
					if (!getIdx_xx((char *) s + 1, idx)) {
						cout << "wrong format (" << j.id << ")" << endl;
						return false;
					}
					if (idx > maxIdx) {
						maxIdx = idx;
//						cout << "\tLxx calc -> maxIdx=" << maxIdx << endl;
					}
				}
			}
		}
	}
	for (auto & i : vecScSet) {
		for (auto & j : i.itms) {
			if (j.id.size() == 3) {
				const char * s = j.id.c_str();
				if ((*s == 'L') || (*s == 'l')) {
					u32 idx = 0;
					if (getIdx_xx((char *) s + 1, idx)) {
						if (idx > maxIdx) {
							maxIdx = idx;
//							cout << "\tLxx calc -> maxIdx=" << maxIdx << endl;
//							cout << "out of range (" << j.id << ")" << endl;
//							return false;
						}
					} else {
						cout << "wrong format \"" << j.id << "\"" << endl;
						return false;
					}
				}
			}
		}
	}
	for (auto & i : vecScCtrl) {
		for (auto & j : i.itms) {
			if (j.id.size() == 3) {
#if DEBUG_PROTOCOL_PARSE_FILE
				cout << "Ctrl Id:" << j.id << endl;
#endif
				const char * s = j.id.c_str();
				if ((*s == 'L') || (*s == 'l')) {
					u32 idx = 0;
					if (getIdx_xx((char *) s + 1, idx)) {
						if (idx > maxIdx) {
							maxIdx = idx;
//							cout << "\tLxx calc -> maxIdx=" << maxIdx << endl;
//							cout << "out of range (" << j.id << ")" << endl;
//							return false;
						}
					} else {
						cout << "wrong format \"" << j.id << "\"" << endl;
						return false;
					}
				}
			}
		}
	}
#if DEBUG
	if (vecBitInit.size() > 0) {
#if DEBUG_DAT_BIT_DAT
		cout << "---- Bits hdl --------------------------" << endl;
#endif
		;
	}
#endif
	for (auto & i : vecBitInit) {
		if (i.bxx.empty())
			continue;
		string bxx = i.bxx;
		bool find = false;
		for(auto & ii : vecBit) {
			if (bxx == ii.bxx) {
				find = true;
				ii.itms.push_back(make_pair(i.id, atoi(i.strLen.c_str())));
				break;
			}
		}
		if (!find) {
			P_ByteBit newItm;
			newItm.bxx = bxx;
			newItm.itms.push_back(make_pair(i.id, atoi(i.strLen.c_str())));
			vecBit.push_back(newItm);
		}
	}
	for (auto & i : vecBit) {
		u32 bits = 0;
		for(auto &ii : i.itms) {
			bits += ii.second;
		}
#if DEBUG_DAT_BIT_DAT
		cout << "bits: ";
		for(auto &j : i.itms)
			cout << j.second << " ";
		cout << endl;
#endif
		if (bits != 8) {
			cout << "bad bit define !" << endl;
			return false;
		}
	}
	vecBitInit.clear();
	L_mem = new u8[maxIdx];
	gDat.commDevSIdAvailTbl.push_back(avi);
	return true;
}



