/*
 * MsgReqFsuLoginInfoRtnDat.h
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */

#ifndef MSGGETFSULOGININFORTNDAT_H_
#define MSGGETFSULOGININFORTNDAT_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"

using namespace std;

class MsgGetFsuLoginInfoRtnDat : public MessagePair {
public:
	MsgGetFsuLoginInfoRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetFsuLoginInfoRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;

	// -- out response msg info

};

#endif /* MSGREQFSULOGININFORTNDAT_H_ */
