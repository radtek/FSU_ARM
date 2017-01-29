/*
 * MsgLoginAck.h
 *
 *  Created on: 2016-4-1
 *      Author: lcz
 */
#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "AppConfig.h"

#ifndef MSGLOGINACK_H_
#define MSGLOGINACK_H_

class MsgLoginAck : public MessagePair
{
public:
	MsgLoginAck(u32 i, u32 o) : MessagePair(i, o, true) {}
	virtual ~MsgLoginAck() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);

private:
	// -- in response msg info
	// -- out request msg info

};


#endif /* MSGLOGINACK_H_ */
