/*
 * MsgGetDatRtnDat.h
 *
 *  Created on: 2016-3-25
 *      Author: lcz
 */

#ifndef MSGGETDATRTNDAT_H_
#define MSGGETDATRTNDAT_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "ReqDevsDats.h"

using namespace std;

class MsgGetDatRtnDat : public MessagePair
{
public:
	MsgGetDatRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetDatRtnDat() {}
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



#endif /* MSGGETDATRTNDAT_H_ */
