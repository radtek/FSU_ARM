/*
 * MsgGetThresholdRtnDat.h
 *
 *  Created on: 2016-3-30
 *      Author: lcz
 */

#ifndef MSGGETTHRESHOLDRTNDAT_H_
#define MSGGETTHRESHOLDRTNDAT_H_


#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "ReqDevsDats.h"

using namespace std;

class MsgGetThresholdRtnDat : public MessagePair
{
public:
	MsgGetThresholdRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetThresholdRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	void addTheDeviceDat(string devId, vector<string> sIds, xmlNodePtr & pDevsNd);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;

	ReqDevsDats m_req;
	// -- out response msg info

};

#endif /* MSGGETTHRESHOLDRTNDAT_H_ */
