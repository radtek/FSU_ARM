/*
 * MsgSetPointRtnDat.h
 *
 *  Created on: 2016-3-29
 *      Author: lcz
 */

#ifndef MSGSETPOINTRTNDAT_H_
#define MSGSETPOINTRTNDAT_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "DevsSemaphores.h"

using namespace std;

class MsgSetPointRtnDat : public MessagePair {
public:
	MsgSetPointRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgSetPointRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- out request msg info
	string FsuId;
	string FsuCode;
	Devs_Semaphores m_req;
	// -- in response msg info

};



#endif /* MSGSETPOINTRTNDAT_H_ */
