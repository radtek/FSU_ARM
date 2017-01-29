/*
 * DB.h
 *
 *  Created on: 2016-3-24
 *      Author: lcz
 */

#ifndef DB_H_
#define DB_H_

#include <vector>
#include <mutex>
#include <list>
#include <sqlite3.h>
#include "define.h"
#include "B_Interface.h"
#include "RunStat.h"


//int DBTest();
//int DBTest2();
//int test_getRcdNum();
#define dl		//cout<< "[++]" << __FUNCTION__ << " ";
#define du		//cout<< "[--]" << __FUNCTION__ << "\n";
struct stDBRecord {
	char ** result;
	int nRow;
	int nCol;
};
class DBFile {
public:
	DBFile();
	~DBFile();
	bool init(string devId, const char *);
	bool createTbl(const char * tblName, const char * itms);
	bool createIdx(const char * idxName);
	bool exec_sql(const char * sql, char * & zErrMsg);
	bool getTblRcdNum(const char * tblName, int &);
	bool getTbl(const char *, char *&);
	bool  beginTransaction();
	bool commitTransaction();
	void rollback();
	void displayTbl(const char *);
	stDBRecord getRcd() { return tmpRcd; }
	void clearResult() {
		sqlite3_free_table(tmpRcd.result);
	}
	int CheckIndexExist(const char * idxName);
public:
	int tempTblExist;
	int tempIdxExist;
	string devId;
private:
	void close() { sqlite3_close(db); }//关闭数据库
	int CheckTableExist(const char * tblName);
private:
	string dbName;
	sqlite3 *db;
	stDBRecord tmpRcd;
	pthread_mutex_t   mutex_lock;
};
class DBHdl {
public:
	struct actAlmItm {
		actAlmItm(string s, string sid, string did) :
			sn(s), semaId(sid), devId(did) {}
		bool same_as(string & devId, string & semaId) {
			return ((semaId == this->semaId)
				&&(devId == this->devId));
		}
		string sn;
		string semaId;
		string devId;
	};
	DBHdl();
	~DBHdl();
	bool init();
	DBFile * findTheAlarmDBFile(string devId);
	DBFile * findTheHistoryDBFile(string devId);
	// ---- alarm ----
	bool isInLastActiveAlmDB(const pair<string, string> &pr, string &);
	bool getLastActiveAlm();
	bool updateActiveAlmDB(vector <BInt::stAlarmDB> &);
	bool saveNewAlarmToUnConfirmedDB(vector<BInt::stDevAlmDB> & v);
	bool batchMove2ConfirmedDB();
	int getUnconfirmedAlmNum(string devId);
	bool getUnconfirmedAlm();
	// ---- history ----
	void upDateHistoryDat(string devId, string devCode,
			vector <BInt::stSemaphore_l> &);
	bool getTheDeviceHistorySemaphores(string devId, vector<string> vecIds,
			vector<BInt::stSemaphore> & out, string startTime, string endTime);
private:
	bool beginDBFileTransaction(DBFile * dbf);
	bool saveOneAlarm2UnConfirmedDB(const BInt::stAlarmDB & alm);
	bool makeTheDeviceAlarmDBFile (DBFile * dbf);
	bool makeTheDeviceHistoryDBFile (DBFile * dbf);
public:
	RunStat runStat;
	vector<BInt::stAlarm> vecAlm;
//	list<pair<string, string>> listActAlm;
	list<actAlmItm> listActAlm;
	DBFile actAlmDB;
	vector<DBFile *> vecAlarmDBs;
	std::mutex mtx_newAlmIn;
private:
	pthread_t tid_chkUnConfirmAlm = 0;
	vector<DBFile *> vecHistoryDBs;
	std::mutex mtx_actAlm;
};

#endif /* DB_H_ */
