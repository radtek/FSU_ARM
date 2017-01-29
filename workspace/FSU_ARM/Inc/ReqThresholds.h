/*
 * ReqThreshold.h
 *
 *  Created on: 2016-3-30
 *      Author: vmuser
 */

#ifndef REQTHRESHOLD_H_
#define REQTHRESHOLD_H_

#include <string>
#include <vector>
#include "xmlHdl.h"
#include "B_Interface.h"

using namespace std;
using namespace BInt;

struct ReqThresholds {
	struct reqDevThreholds {
		string devId;	// char[14]
		string code;	// char[14]
		vector<stThreshold> thrs;
	};
	vector<reqDevThreholds> devs;
	void setDat(xmlXPathObjectPtr p);
	void clearDat();
};
#endif /* REQTHRESHOLD_H_ */
