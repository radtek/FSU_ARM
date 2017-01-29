/*
 * MsgLogoutAck.h
 *
 *  Created on: 2016-4-1
 *      Author: lcz
 */

#ifndef MSGLOGOUTACK_H_
#define MSGLOGOUTACK_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"


class MsgLogoutAck : public MessagePair
{
public:
	MsgLogoutAck(u32 i, u32 o) : MessagePair(i, o, true) {}
	virtual ~MsgLogoutAck() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);

private:
	// -- in response msg info
	//string Result;
	// -- out request msg info

};

#endif /* MSGLOGOUTACK_H_ */
