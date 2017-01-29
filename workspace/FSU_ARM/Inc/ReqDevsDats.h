/*
 * ReqDevsDats.h
 *
 *  Created on: 2016-3-27
 *      Author: vmuser
 */

#ifndef REQDEVSDATS_H_
#define REQDEVSDATS_H_

#include <string>
#include <vector>

using namespace std;

struct ReqDevsDats {
	struct reqDevIds {
		bool all;
		string devId;	// char[14]
		string code;	// char[14]
		vector<string> ids;
	};
	bool all;
	vector<reqDevIds> devs;
	void setDat(xmlXPathObjectPtr p);
	void clearDat();
};


#endif /* REQDEVSDATS_H_ */
