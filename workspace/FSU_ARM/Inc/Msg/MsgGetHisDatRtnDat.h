/*
 * MsgGetHisDatRtnDat.h
 *
 *  Created on: 2016-3-27
 *      Author: lcz
 */

#ifndef MSGGETHISDATRTNDAT_H_
#define MSGGETHISDATRTNDAT_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"
#include "ReqDevsDats.h"

using namespace std;

class MsgGetHisDatRtnDat : public MessagePair {
public:
	MsgGetHisDatRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgGetHisDatRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	void addTheDeviceDat(string devId, vector<string> sIds, xmlNodePtr & pDevsNd);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	string startTime;
	string endTime;

	ReqDevsDats m_req;

	// -- out response msg info

};



#endif /* MSGGETHISDATRTNDAT_H_ */
