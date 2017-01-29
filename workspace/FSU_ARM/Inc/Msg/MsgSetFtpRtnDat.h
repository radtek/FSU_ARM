/*
 * MsgSetFtpRtnDat.h
 *
 *  Created on: 2016-3-31
 *      Author: lcz
 */

#ifndef MSGSETFTPRTNDAT_H_
#define MSGSETFTPRTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"

class MsgSetFtpRtnDat : public MessagePair {
public:
	MsgSetFtpRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgSetFtpRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	string UserName;
	string Password;
	// -- out response msg info

};

#endif /* MSGSETFTPRTNDAT_H_ */
