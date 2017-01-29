/*
 * MsgSendAlarmAck.h
 *
 *  Created on: 2016-4-1
 *      Author: vmuser
 */

#ifndef MSGSENDALARMACK_H_
#define MSGSENDALARMACK_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "B_Interface.h"

using namespace std;
using namespace BInt;

class MsgSendAlarmAck : public MessagePair
{
public:
	MsgSendAlarmAck(u32 i, u32 o) : MessagePair(i, o, true) {}
	virtual ~MsgSendAlarmAck() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in response msg info
	string Result;
	// -- out request msg info

};
#endif /* MSGSENDALARMACK_H_ */
