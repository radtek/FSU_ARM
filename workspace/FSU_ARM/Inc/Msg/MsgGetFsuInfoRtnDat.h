/*
 * MsgGetFsuInfoRtnDat.h
 *
 *  Created on: 2016-3-31
 *      Author: vmuser
 */

#ifndef MSGGETFSUINFORTNDAT_H_
#define MSGGETFSUINFORTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"

class MsgGetFsuInfoRtnDat : public MessagePair {
public:
	MsgGetFsuInfoRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetFsuInfoRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	// -- out response msg info
	string cpuOccRate;
	string memOccRate;
};

#endif /* MSGGETFSUINFORTNDAT_H_ */
