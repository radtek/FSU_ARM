/*
 * MsgTimeCheckRtnDat.h
 *
 *  Created on: 2016-3-31
 *      Author: vmuser
 */

#ifndef MSGTIMECHECKRTNDAT_H_
#define MSGTIMECHECKRTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"

class MsgTimeCheckRtnDat : public MessagePair {
public:
	MsgTimeCheckRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgTimeCheckRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string Year;
	string Month;
	string Day;
	string Hour;
	string Minute;
	string Second;
	// -- out response msg info

};

#endif /* MSGTIMECHECKRTNDAT_H_ */
