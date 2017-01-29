  #ifndef _DEFINE_H_
#define _DEFINE_H_

#include <iostream>
#include <string>
#include <vector>
#include<sys/prctl.h>
#include "debug.h"
#include "Log.h"

using namespace std;

typedef const char * PZStr;

typedef unsigned int 	u32;
typedef unsigned short 	u16;
typedef unsigned char 	u8;
typedef signed int 		s32;
typedef signed short 	s16;
typedef signed char 		s8;

typedef const char *	cpc;

struct stDateTimeStr {
	string year;
	string mon;
	string day;
	string hour;
	string min;
	string sec;
};
struct portConfigItm {
	string ttyType;
	string ttyPort;
	string devId;
};
struct devIdentifyItm {
	int devModel;
	u32 way;
	string outStr;
	u32 rtnLen;
	u32 startPos;
	vector <pair<string, u32>> ids;
//	u32 protocolFormat;
};
struct portScanTblItm {
	int devType;
	vector <devIdentifyItm> identifyTbl;
};
struct dev18CfgItm {
	u8 addr;
	string sc_devId;
	string semaId;
};
struct extSemaDefItm {
	u8 addr;
	u32 ch;
	string name;
	string devId;
	string semaId;
};
struct semaRelCfgItm {
	u8 addr;
	string src_semaId;
	string sc_devId;
	string sc_semaId;
};

template<typename T, size_t N>
size_t countof(T (&array)[N]) {
	return N;
}
float str2Float(string str);
int str2Int(string str);
void IntToString (std::string& out, const int value);
bool setSysTime(stDateTimeStr &tm);
float GetHostCPURate();
float getMemOccupyRate();

bool GetMyCPURate(string & out);
bool GetMyMemRate(string & out);

string floatToString(double fZ,const int slen,const int alen);
void getAbsTime(timespec & t, u32 timeout_ms);
double getDouble(const char * s);
bool isStrDatOk(string str);
bool checkAndChangeDateFormat (string & s);

#define  SET_THRD_NAME() prctl(PR_SET_NAME, __func__)
template < class T >
void ClearVector( vector< T >& vt )
{
    vector< T > vtTemp;
    vtTemp.swap( vt );
}
template <typename T>
void ClearPointerVector(T &t) {
	cout << __FUNCTION__ << "size = " << t.size() << endl;
//	for(auto * p : t) {
//		if (p != NULL) {
//			delete p;
//			p = NULL;
//		}
//	}
//	t.clear();

	for (typename T::iterator it = t.begin(); it != t.end(); it ++) {
	    if (NULL != *it) {
	        delete *it;
	        *it = NULL;
	    }
	}
	ClearVector(t);
}

char Char2Num(char c);
u8 Ch2_2_Num(char c1,char c2);
bool readCsv(vector<vector<string> > & data, string & filename);
bool writeCsv(string & filename, const vector<vector<string> > &vvStr);
int IsFolderExist(const char* path);
int IsFileExist(const char* path);
int ipaddr(string & ip);
void printAsc(unsigned char * pBuf, u32 len);
void printHex(unsigned char * pBuf, u32 len);
void printHexNo_n(unsigned char * pBuf, u32 len);

void outputInitItm(vector<string> & v);
bool skipTheLine(const vector<string> &in);
bool checkQuit(const vector<string> &in, u32 line, const string & fn, u32 min);
//template <typename T>
void InsertSort(u32 arr[],int count);
string findFileNameOfPath(const string & path);
#endif

