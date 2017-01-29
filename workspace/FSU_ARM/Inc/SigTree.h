/*
 * SigTree.h
 *
 *  Created on: 2016-5-3
 *      Author: lcz
 */

#ifndef SIGTREE_H_
#define SIGTREE_H_

#include <vector>
#include <string.h>

using namespace std;

typedef string SigId;

class Semaphore {	// 监控点
	SigId  id;
	string type;
	string measVal;
	string setupVal;
	string status;
};
class Threshold {	// 监控点门限
	SigId  id;
	string type;
	string val;		// threshold
	string absVal;	// absolute
	string relVal;	// relative
	string status;
};

template <typename T> class DevSig {
	string devCode;
	string devId;
	vector<T> vecSig;
	void pushSig(T s) {
		vecSig.push_back(s);
	}
};
template <typename T> class SigTree {
public:
	vector<DevSig<T> > treeSig;
	void pushDev(DevSig<T> d) {
		treeSig.push_back(d);
	}
};


#endif /* SIGTREE_H_ */
