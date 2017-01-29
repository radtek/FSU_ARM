/*
 * DevComDatParse.h
 *
 *  Created on: 2016-5-23
 *      Author: lcz
 */

#ifndef DEVCOMDATPARSE_H_
#define DEVCOMDATPARSE_H_
#include <string>
#include <vector>

using namespace std;


struct ID_VAL_PAIR {
	string id;
	string val;
};

typedef void (*PFN_PreHdl) (char * s);
typedef bool (*PFN_CRC) (const char * s);

#if 0
//抽象模板，并定义了一个模板方法。
class AbstractParse {
public:
    AbstractParse();
    //~AbstractParse();
    bool init(const char * );
    //模板方法
    void run(char * s);
    //抽象方法
    PFN_PreHdl pPreHdl;// 预处理
    PFN_CRC pCrc;// CRC校验

private:
    void getDatList(vector <ID_VAL_PAIR> &);
public:
    void exit();
private:
    void * hDynLib;
    bool bDllLoaded;
};
#endif
////实现基类所定义的抽象方法
//class P13633DevParse:public AbstractParse {
//public:
//	P13633DevParse() {};
//    ~P13633DevParse() {};
//    //实现基类定义的抽象行为
//    virtual void preHdl(string &str);
//    virtual bool crc(const string & inStr);
// //   virtual void getDatList(vector <ID_VAL_PAIR> & out);
//private:
//};



#endif /* DEVCOMDATPARSE_H_ */
