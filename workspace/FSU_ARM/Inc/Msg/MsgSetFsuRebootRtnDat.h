/*
 * MsgSetFsuRebootRtnDat.h
 *
 *  Created on: 2016-3-31
 *      Author: vmuser
 */

#ifndef MSGSETFSUREBOOTRTNDAT_H_
#define MSGSETFSUREBOOTRTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"

class MsgSetFsuRebootRtnDat : public MessagePair {
public:
	MsgSetFsuRebootRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgSetFsuRebootRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	// -- out response msg info

};
#endif /* MSGSETFSUREBOOTRTNDAT_H_ */
