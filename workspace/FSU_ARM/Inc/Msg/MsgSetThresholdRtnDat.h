/*
 * MsgSetThresholdRtnDat.h
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */

#ifndef MSGSETTHRESHOLDRTNDAT_H_
#define MSGSETTHRESHOLDRTNDAT_H_

#include "define.h"
#include <string>
#include "Message.h"
#include "ReqThresholds.h"

using namespace std;

class MsgSetThresholdRtnDat : public MessagePair
{
public:
	MsgSetThresholdRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgSetThresholdRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string FsuId;
	string FsuCode;

	ReqThresholds m_req;
	// -- out response msg info

};
#endif /* MSGSETTHRESHOLDRTNDAT_H_ */
