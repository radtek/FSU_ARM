/*
 * DevComDatParse.cpp
 *
 *  Created on: 2016-5-23
 *      Author: vmuser
 */

#include <iostream>
#include <dlfcn.h>
#include "AppConfig.h"
#include "DevComDatParse.h"

using namespace std;
#if 0
AbstractParse::AbstractParse() {
	hDynLib = 0;
	bDllLoaded = false;
};
void AbstractParse::exit() {
	dlclose(hDynLib);
}
void AbstractParse::run(char * s) {
	if (!bDllLoaded)
		return;
	if (*pPreHdl != 0)
		(*pPreHdl)(s);
	if (*pCrc != 0) {
		if((*pCrc)(s))
			return;
	}
    vector <ID_VAL_PAIR> vec;
    getDatList(vec);
    //valConvert(vec);	// 数值变换
    //save2DB(vec);
}

void AbstractParse::getDatList(vector <ID_VAL_PAIR> &) {

}

bool AbstractParse::init(const char * s) {
	string p = gDat.exePath + string(s);//string("libp13633.so");
    PFN_PreHdl f1;
    PFN_CRC f2;
    void * hDynLib = 0;
	bDllLoaded = false;
    hDynLib = dlopen(p.c_str(), RTLD_LAZY);
    if (hDynLib) {
    	pPreHdl = (PFN_PreHdl)dlsym(hDynLib, "preHdl");
    	if (0 == pPreHdl) {
    		printf("Get DLL function error ---> preHdl.\n");
    		return false;
    	}
    	pCrc = (PFN_CRC)dlsym(hDynLib, "crc");
    	if (0 == pPreHdl) {
    		printf("Get DLL function error ---> crc.\n");
    		return false;
    	}
    } else {
    	printf("DLL load error ---> %s.\n", s);
    	return false;
    }
	bDllLoaded = true;
    return true;
}

#endif


