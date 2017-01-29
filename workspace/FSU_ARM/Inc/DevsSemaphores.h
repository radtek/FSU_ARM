/*
 * DevsSemaphores.h
 *
 *  Created on: 2016-3-29
 *      Author: lcz
 */

#ifndef DEVS_SEMAPHORES_H_
#define DEVS_SEMAPHORES_H_

#include <string>
#include <vector>
#include "xmlHdl.h"
#include "define.h"
#include "B_Interface.h"

using namespace std;
using namespace BInt;

struct Devs_Semaphores {
	struct DevSemaphores {
		string devId;	// char[14]
		string code;	// char[14]
		vector<stSemaphore_l> sems;
	};
	Devs_Semaphores & operator=(const Devs_Semaphores & r);
	vector<DevSemaphores> devs;
	void setDat(xmlXPathObjectPtr p);
	void clearDat();
};

#endif /* REQSETTINGS_H_ */
