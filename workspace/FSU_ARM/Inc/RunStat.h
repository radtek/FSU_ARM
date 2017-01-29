/*
 * RunStat.h
 *
 *  Created on: 2017年1月27日
 *      Author: root
 */

#ifndef INC_RUNSTAT_H_
#define INC_RUNSTAT_H_

class RunStat {
public:
	bool isRunning() {
		bool rtn = false;
		mtx.lock();
		rtn = running;
		mtx.unlock();
		return rtn;
	}
	void setRun(bool b) {
		mtx.lock();
		running = b;
		mtx.unlock();
	}
private:
	bool running;
	std::mutex mtx;
};

#endif /* INC_RUNSTAT_H_ */
