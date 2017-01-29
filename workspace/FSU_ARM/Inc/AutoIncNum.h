/*
 * AutoIncNum.h
 *
 *  Created on: 2016年9月10日
 *      Author: root
 */

#ifndef SRC_AUTOINCNUM_H_
#define SRC_AUTOINCNUM_H_
#include <mutex>

class AutoIncNum {
public:
	AutoIncNum();
	bool init();
	bool add(unsigned int n, unsigned int &out);
	string fn;
	const char *pf;
	unsigned int  idx;
	std::mutex mtx;
};

#endif /* SRC_AUTOINCNUM_H_ */
