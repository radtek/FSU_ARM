/*
 * HFAlarm.cpp
 *
 *  Created on: 2016-7-4
 *      Author: lcz
 */

#include "time.h"
#include "HFAlarm.h"

const int HFA_ACTIVE_TIME = 1800;
const int HFA_DEACTIVE_TIME = 1800;//1800;;

bool HFAlarm::addNewAndCheckActive(const time_t t) {
	lastIsOn = true;
//	cout << "####### full 1" << endl;
	dat.addNew(t);
	cout << "-in " << __FUNCTION__ << ":cnt=";
	cout << dat.Count() << endl;
//	cout << "####### full 2" << endl;
	if (dat.isFull()) {
//		cout << "####### full 3" << endl;
		if (difftime(dat.end(), dat.first()) < HFA_ACTIVE_TIME) {
//			cout << "########### full and HF Active !" << endl << endl;
			return true;
		}
	}
	return false;
}
void HFAlarm::setOnState() {
//	cout << "in " << __FUNCTION__ << endl;
	lastIsOn = true;
}
bool HFAlarm::isDisactive() {
//	cout << "in " << __FUNCTION__ << endl;
	if (lastIsOn) {
		t_On2Off = time(NULL);
		lastIsOn = false;
	}
	if (dat.isFull()) {
		int dif = difftime(time(NULL), t_On2Off);
		cout << "dif=" << dif << endl;
		if (dif > HFA_DEACTIVE_TIME) {
			dat.reset();

			return true;
		}
	}
	return false;
}




