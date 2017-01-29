/*
 * AlarmHdl.h
 *
 *  Created on: 2016-6-14
 *      Author: lcz
 */

#ifndef ALARMHDL_H_
#define ALARMHDL_H_

#include <vector>
#include "B_Interface.h"

using namespace std;
using namespace BInt;



//void AlarmlinkByIR();			// 红外信号
//void AlarmLinkByDoorOpen();		// 开门信号

//void AlarmLink(bool openDoor, bool doorOpen);

void getUnReportAlmList(vector<stAlarm> &);

#endif /* ALARMHDL_H_ */
