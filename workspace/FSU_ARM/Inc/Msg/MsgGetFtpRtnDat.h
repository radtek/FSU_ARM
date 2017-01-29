/*
 * MsgGetFtpRtnDat.h
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */

#ifndef MSGGETFTPRTNDAT_H_
#define MSGGETFTPRTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"

class MsgGetFtpRtnDat : public MessagePair {
public:
	MsgGetFtpRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetFtpRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	// -- out response msg info

};

#endif /* MSGGETFTPRTNDAT_H_ */
