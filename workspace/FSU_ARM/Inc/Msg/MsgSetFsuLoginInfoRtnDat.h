/*
 * MsgSetFsuLoginInfoRtnDat.h
 *
 *  Created on: 2016-3-30
 *      Author: vmuser
 */

#ifndef MSGSETFSULOGININFORTNDAT_H_
#define MSGSETFSULOGININFORTNDAT_H_

#include "define.h"
#include <string>
#include <vector>
#include "Message.h"

using namespace std;

class MsgSetFsuLoginInfoRtnDat : public MessagePair {
public:
	MsgSetFsuLoginInfoRtnDat(u32 i, u32 o) : MessagePair(i, o, false) {}
	virtual ~MsgSetFsuLoginInfoRtnDat() {}
	int parseXmlInfoPart(string &xmlStr);
	int makeOutXml(bool bSuccess, string &str2SC);
private:
	// -- in request msg info
	string fsuId;
	string fsuCode;
	string IPSecUser;
	string IPSecPWD;
	string IPSecIP;
	string SCIP;
	struct dev {
		string Id;
		string Code;
	};
	vector<dev> m_devs;

	// -- out response msg info
};

#endif /* MSGSETFSULOGININFORTNDAT_H_ */
