/*
 * HFAlarm_H_.h
 *
 *  Created on: 2016-7-4
 *      Author: lcz
 */

#ifndef HF_ALARM_H_
#define HF_ALARM_H_

#include "define.h"
#include "FixSizeFIFO.h"

/*HFAlarm.h*/

class HFAlarm {
public:
	bool addNewAndCheckActive(const time_t);
	bool isDisactive();
	void setOnState();
	void reset() {		dat.reset();	}
private:
	FixSizeFIFO <time_t, 6> dat;	// std is 6 !
	time_t t_On2Off;
	bool lastIsOn = false;
};


#endif /* HF_ALARM_H_ */
