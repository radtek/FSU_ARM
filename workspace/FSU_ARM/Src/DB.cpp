/*
 * DB.cpp
 *
 *  Created on: 2016-3-24
 *      Author: lcz
 */
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <sys/stat.h>
#include "define.h"
#include "debug.h"
#include "DB.h"
#include "Message.h"
#include "soap.h"
#include "AppConfig.h"
#include "B_Interface.h"
#include "Pipe.h"
#include "LoginLogout.h"

using namespace std;

DBHdl DB;
extern GlobalDat gDat;
extern SCCallProxy scProxy;

DBFile::DBFile() {
	db = 0;
	tempTblExist = 0;
	tempIdxExist = 0;
	pthread_mutex_init(&mutex_lock, NULL);
}
DBFile::~DBFile() {
//	clearResult();
	close();
	pthread_mutex_destroy(&mutex_lock);
}
bool DBFile::init(string _devId, const char * fileName) {
	if (SQLITE_OK != sqlite3_open(fileName, &db)) {
		char err[256];
		sprintf(err, "Database error : %s.", sqlite3_errmsg(db));
		log(err);
		cout << err << endl;
		close();
		return false;
	}
	devId = _devId;
	dbName = string(fileName);
	return true;
}
int sqlite3_callback_func_tbl_Exist(void* pHandle, int iRet, char** szSrc,
		char** szDst) {
	DBFile * pDB = (DBFile *) pHandle;
	if (1 == iRet) {
		pDB->tempTblExist = atoi(*(szSrc)); //此处返回值为查询到同名表的个数，没有则为0，否则大于0
		// szDst 指向的内容为"count(*)"
	}
	return 0; //返回值一定要写，否则下次调用 sqlite3_exec(...) 时会返回 SQLITE_ABORT
}
int sqlite3_callback_func_idx_Exist(void* pHandle, int iRet, char** szSrc,
		char** szDst) {
	DBFile * pDB = (DBFile *) pHandle;
	if (1 == iRet) {
		pDB->tempIdxExist = atoi(*(szSrc)); //此处返回值为查询到同名表的个数，没有则为0，否则大于0
		// szDst 指向的内容为"count(*)"
	}
	return 0; //返回值一定要写，否则下次调用 sqlite3_exec(...) 时会返回 SQLITE_ABORT
}
int DBFile::CheckTableExist(const char * tblName) {
	char* zErrMsg = NULL;
	char sql[255];
	tempTblExist = 0;
	sprintf(sql,
			"select count(*) from sqlite_master where type='table' and name='%s'",
			tblName);
	//void *pHandle = ***;
	if (SQLITE_OK
			!= sqlite3_exec(db, sql, &sqlite3_callback_func_tbl_Exist, this,
					&zErrMsg)) {
		cout << "check table exist error: " << zErrMsg << endl;
	}
	//回调函数无返回值，则此处第一次时返回SQLITE_OK， 第n次会返回SQLITE_ABORT

	return tempTblExist;
}
int DBFile::CheckIndexExist(const char * idxName) {
	char* zErrMsg = NULL;
	char sql[255];
	tempIdxExist = 0;
	sprintf(sql,
			"select count(*) from sqlite_master where type='index' and name='%s'",
			idxName);
	//void *pHandle = ***;
	if (SQLITE_OK
			!= sqlite3_exec(db, sql, &sqlite3_callback_func_idx_Exist, this,
					&zErrMsg)) {
		cout << "check index exist error: " << zErrMsg << endl;
	}
	//回调函数无返回值，则此处第一次时返回SQLITE_OK， 第n次会返回SQLITE_ABORT

	return tempIdxExist;
}
bool DBFile::createTbl(const char * tblName, const char * itms) {
	bool rtn = false;
	if (db) {
		if (CheckTableExist(tblName) != 1) {
			char sql[255];
			sprintf(sql, "create table %s (%s)", tblName, itms);
			char* zErrMsg = NULL;
			if (SQLITE_OK == sqlite3_exec(db, sql, NULL, 0, &zErrMsg))
				rtn = true;
			else
				cout << "create table [" << tblName << "] error: " << zErrMsg
						<< endl;
		} else
			return true;
	}
	return rtn;
}
bool DBFile::exec_sql(const char * sql, char * & zErrMsg) {
	bool rtn;
	pthread_mutex_lock(&mutex_lock);
	dl
	rtn = (SQLITE_OK == sqlite3_exec(db, sql, 0, 0, &zErrMsg));
	pthread_mutex_unlock(&mutex_lock);
	du
	return rtn;
}
bool DBFile::getTbl(const char * sql, char *& zErr) {
	bool rtn = false;
	pthread_mutex_lock(&mutex_lock);
	dl
	rtn = (SQLITE_OK
			== sqlite3_get_table(db, sql, &tmpRcd.result, &tmpRcd.nRow,
					&tmpRcd.nCol, &zErr));
	pthread_mutex_unlock(&mutex_lock);
	du
	return rtn;
}
bool DBFile::getTblRcdNum(const char * tblName, int & num) {
	char *zErrMsg = 0;
	char sql[255];
	bool rtn = false;
	sprintf(sql, "select count(*) from %s", tblName);
	if (getTbl(sql, zErrMsg)) {
		num = atoi(*(tmpRcd.result + 1));
		rtn = true;
	}
	clearResult();
	return rtn;
}
void DBFile::displayTbl(const char * tblName) {
	char sql[255];
	sprintf(sql, "select * from %s", tblName);
	char *zErrMsg = 0;
	if (getTbl(sql, zErrMsg)) {
		for (int r = 0; r < tmpRcd.nRow + 1; ++r) {
			for (int c = 0; c < tmpRcd.nCol; ++c) {
				cout << tmpRcd.result[r * tmpRcd.nCol + c] << "|";
			}
			cout << endl;
		}
	}
	clearResult();
}
bool DBFile::beginTransaction() {
	char *zErrMsg = 0;
	if (!exec_sql("begin transaction;", zErrMsg)) {
		cout << zErrMsg << endl;
		log(zErrMsg);
		return false;
	}
	return true;
}
bool DBFile::commitTransaction() {
	char *zErrMsg = 0;
	if (!exec_sql("commit transaction;", zErrMsg)) {
		cout << zErrMsg << endl;
		log(zErrMsg);
		return false;
	}
	return true;
}
void DBFile::rollback() {
	char *zErrMsg = 0;
	if (!exec_sql("rollback transaction;", zErrMsg)) {
		log(zErrMsg);
	}
}
//---------------------------------------------------------------------------
extern LoginHdl loginHdl;
void * ReportUnconfirmAlmThrd(void * param) {
	SET_THRD_NAME();
	DBHdl * pDb = (DBHdl *) param;
	string str2SC, strSCResponse;
	static unsigned int cnt = 0;
	while (pDb->runStat.isRunning()) {
		threadDebug
		if (!loginHdl.isLogin()) {
			sleep(10);
		} else {
			pDb->mtx_newAlmIn.lock();
			for (auto & db : pDb->vecAlarmDBs) {
//				step(0);
				u32 num = pDb->getUnconfirmedAlmNum(db->devId);
//				step(1);
				if (num > 0) {		//
					cout << "\n\n\t######## get unconfirmed alarm num is " << num << endl << endl;
					if (scProxy.getAlarmPak(str2SC) > 0) {	// XXX
//						step(2);
						for (cnt = 0; cnt < 3;) {
//							step(3);
							if (SCGetter_Invoke(str2SC, strSCResponse)) { // call SC SVC. get all alm save 2 vecAlm
//								step(4);
								if (MessageMgr::hdlScResponse(strSCResponse) == 1) { // alarm ACK OK.
	//								cout << "******************* Alm Ack OK *******************" << endl;
									pDb->batchMove2ConfirmedDB();	// batch hdl
									break;
								}
							}
//							else {
//								cnt++;
//								sleep(5);
//							}
						}
						break;
//						if (cnt >= 3) {
//							loginHdl.setLoginState(false);
//						}
					}
				}
			}

			pDb->mtx_newAlmIn.unlock();
			sleep(2);
		}
	}
	return ((void *) 0);
}
bool DBHdl::makeTheDeviceAlarmDBFile (DBFile * dbf) {
	if (!dbf->createTbl("unconfirm_alarm",
			"[sn],[id],[fsuId],[fsuCode],[deviceId],[deviceCode],[alm_dateTime],[alm_leve],[alm_flag],[alm_desc]"))
		return false;
	if (!dbf->createTbl("confirmed_alarm",
			"[sn],[id],[fsuId],[fsuCode],[deviceId],[deviceCode],[alm_dateTime],[alm_leve],[alm_flag],[alm_desc]"))
		return false;
	return true;
}
bool DBHdl::makeTheDeviceHistoryDBFile (DBFile * dbf) {
	if (!dbf->createTbl("history_dat",
		"[deviceId], [devceCode], [semId] integer, [type], [mesuredVal], [setupVal], [status],"
		" [recordTime] TimeStamp NOT NULL DEFAULT (datetime('now','localtime'))"))
		return false;
	const char* s = "CREATE INDEX idx_id on history_dat(semId)";
	char * zErr = 0;
	if (dbf->CheckIndexExist("idx_id") != 1)
		return dbf->exec_sql(s, zErr);
	return true;
}
bool DBHdl::init() {
	string dir_db = gDat.exePath + "/db";
	if (IsFileExist(dir_db.c_str()) == 0) {
		if (mkdir(dir_db.c_str(), 0777) != 0) {
			cout << "Can not make dir ./db !" << endl;
			return false;
		}
	}
	string dir_db_alm = dir_db + "/alarm";
	if (IsFileExist(dir_db_alm.c_str()) == 0) {
		if (mkdir(dir_db_alm.c_str(), 0777) != 0) {
			cout << "Can not make dir ./db/alarm !" << endl;
			return false;
		}
	}
	string dir_db_history = dir_db + "/history";
	if (IsFileExist(dir_db_history.c_str()) == 0) {
		if (mkdir(dir_db_history.c_str(), 0777) != 0) {
			cout << "Can not make dir ./db/history !" << endl;
			return false;
		}
	}
	for (auto & dev : gDat.vecDev) {
		string dir_dev = dir_db_alm + "/" + dev.Id;
		if (IsFileExist(dir_dev.c_str()) == 0) {
			if (mkdir(dir_dev.c_str(), 0777) != 0) {
				cout << "Can not make dir ./db/alarm/" << dev.Id << " !" << endl;
				return false;
			}
		}
	}
	for (auto & dev : gDat.vecDev) {
		string dir_dev = dir_db_history + "/" + dev.Id;
		if (IsFileExist(dir_dev.c_str()) == 0) {
			if (mkdir(dir_dev.c_str(), 0777) != 0) {
				cout << "Can not make dir ./db/history/" << dev.Id << " !" << endl;
				return false;
			}
		}
	}
	string actAlmDBFileName = gDat.exePath + "db/" + "actAlm.db3";
	if(!actAlmDB.init(string("all"), actAlmDBFileName.c_str()))
		return false;
	if (!actAlmDB.createTbl("active_alarm", "[sn],[semId],[deviceId]"))
		return false;
	for (const auto & dev : gDat.vecDev) {
		string dbName1 = gDat.exePath + "db/alarm/" + dev.Id + "/dat.db3";
//		cout << "init dbName = " << dbName1 << endl;
		DBFile * dbf = new DBFile();
		if (!dbf->init(dev.Id, dbName1.c_str()))
			return false;
		if (!makeTheDeviceAlarmDBFile(dbf))
			return false;
		vecAlarmDBs.push_back(dbf);

		string dbName2 = gDat.exePath + "db/history/" + dev.Id + "/dat.db3";
		dbf = new DBFile();
		if (!dbf->init(dev.Id, dbName2.c_str()))
			return false;
		if(!makeTheDeviceHistoryDBFile(dbf))
			return false;
		vecHistoryDBs.push_back(dbf);
	}

	runStat.setRun(true);

	int ret;
	ret = pthread_create(&tid_chkUnConfirmAlm, NULL, ReportUnconfirmAlmThrd, this);
	if (ret < 0)
		return false;

	return true;
}
DBFile * DBHdl::findTheAlarmDBFile(string devId) {
	for (auto & pDbf : vecAlarmDBs) {
		if (pDbf->devId == devId) {
			return pDbf;
		}
	}
	return NULL;
}
DBFile * DBHdl::findTheHistoryDBFile(string devId) {
	for (auto & pDbf : vecHistoryDBs) {
		if (pDbf->devId == devId) {
			return pDbf;
		}
	}
	return NULL;
}
DBHdl::DBHdl() {
	vecAlarmDBs.clear();
	vecHistoryDBs.clear();
	runStat.setRun(false);
	vecAlm.clear();
}
DBHdl::~DBHdl() {
	runStat.setRun(false);
	pthread_join(tid_chkUnConfirmAlm, NULL);
	WHERE_AM_I
	ClearPointerVector(vecAlarmDBs);
	ClearPointerVector(vecHistoryDBs);
}
int DBHdl::getUnconfirmedAlmNum(string devId) {
	DBFile * db = findTheAlarmDBFile(devId);
	if (db) {
		int n;
		if (db->getTblRcdNum("unconfirm_alarm", n))
			return n;
	}
	return 0;
}
bool DBHdl::isInLastActiveAlmDB(const pair<string, string> &pr, string &sn) {
	mtx_actAlm.lock();
	for (auto & i : listActAlm) {
		if ((i.semaId == pr.second) && (i.devId == pr.first)) {
			sn = i.sn;
			mtx_actAlm.unlock();
			return true;
		}
	}
	mtx_actAlm.unlock();
	return false;
}
bool DBHdl::getLastActiveAlm() {
	char *zErrMsg = 0;
	mtx_actAlm.lock();
	listActAlm.clear();
	if (actAlmDB.getTbl("select * FROM active_alarm", zErrMsg)) {
		stDBRecord rcd = actAlmDB.getRcd();
		if (rcd.nRow > 0) {
			for (int r = 0; r < rcd.nRow + 1; ++r) {
#if DEBUG_ALM_CHK_PROC
				cout << "\t|";
				for (int c = 0; c < rcd.nCol; ++c) {
					string s = string(rcd.result[r * rcd.nCol + c]);
					cout << s << "|";
				}
				cout << endl;
#endif
				if (r != 0)		// skip 1st line .
					listActAlm.push_back(
							actAlmItm(	rcd.result[r * rcd.nCol + 0],
										rcd.result[r * rcd.nCol + 1],
										rcd.result[r * rcd.nCol + 2]));
			}
		}
		actAlmDB.clearResult();
	} else {
		cout << zErrMsg << endl;
		log("select * FROM active_alarm - error, in updateActiveAlmDB()");
		actAlmDB.clearResult();
		mtx_actAlm.unlock();
		return false;
	}
//#if DEBUG_ALM_CHK_END
//	cout << "\n*******************************************************" << endl;
//	for (const auto & a : listActAlm) {
//		cout << "*\t" << a.first << "\t|" << a.second << "\t*" << endl;
//	}
//	cout << "*******************************************************\n" << endl;
//#endif

	mtx_actAlm.unlock();
	return true;
}
//}
bool DBHdl::saveOneAlarm2UnConfirmedDB(const stAlarmDB & alm) {
	char *zErrMsg = 0;
#if DEBUG_DB_TABLE
	cout << "\tsave Alm(" << alm.sn << "," << alm.deviceId << "," << alm.id << ")" << endl;
#endif
	DBFile * dbf = findTheAlarmDBFile(alm.deviceId);
	if (dbf) {
//		if (!dbf->beginTransaction())
//			return false;
		char sql[256];
		string fsuId = gDat.cfgs[GIDX_FSUID];
		string fsuCode = gDat.cfgs[GIDX_FSUCODE];
		char sn[11];
		sprintf(sn, "%010d", alm.sn);
		sprintf(sql, "INSERT INTO unconfirm_alarm VALUES"
				"('%s','%s','%s','%s','%s','%s',datetime('now'),'%s','%s','%s')",
				sn,alm.id.c_str(), fsuId.c_str(), fsuCode.c_str(),
				alm.deviceId.c_str(), alm.deviceCode.c_str(), /*alm.time.c_str(),*/
				alm.level.c_str(), alm.flag.c_str(), alm.desc.c_str());
#if DEBUG_SQL
		cout << "SQL:" << sql << endl;
#endif
		if (!dbf->exec_sql(sql, zErrMsg)) {
			log("insert to database err, in saveNewAlarmToConfirmedDB()");
			dbf->rollback();
			return false;
		}
//		if (!dbf->commitTransaction())
//			return false;

// ------------------------- sync listActAlm ------------------------------------------------
		mtx_actAlm.lock();
		if (alm.flag == gDat.almFlag[0]) {
			listActAlm.push_back(actAlmItm(sn, alm.id, alm.deviceId));
			cout << "\tlistActAlm push_back(" << alm.deviceId << "," << alm.id << ")" << endl;
		} else {
			listActAlm.remove_if([&](const actAlmItm & i) {	return (i.sn == sn);});
			cout << "\tlistActAlm remove(" << alm.deviceId << "," << alm.id << ")" << endl;
		}
		mtx_actAlm.unlock();
	} else {
		cout << "can't find DB file (" << alm.deviceId << "!";
		return false;
	}
	return true;
}
bool DBHdl::saveNewAlarmToUnConfirmedDB(vector<stDevAlmDB> & v) {
	mtx_newAlmIn.lock();
	for (const auto & dev : v) {
#if DEBUG_ALM_CHK_DETAIL
		cout << "devId=" << dev.devId << endl;
#endif
		for (const auto & a : dev.v) {
#if DEBUG_ALM_CHK_DETAIL
			cout << "\tDevId[" << dev.devId << "]";
			cout << "id[" << a.id << "], flag = " << a.flag;
			cout << "\"" << a.desc << "\"" << endl;
#endif
			if (!saveOneAlarm2UnConfirmedDB(a))
				continue;
		}
	}
#if DEBUG_ALM_CHK_DETAIL
	mtx_actAlm.lock();

	for (const auto & a : listActAlm) {
		cout << "* " << a.sn << "\t|" << a.devId << "\t|" << a.semaId << "\t*" << endl;
	}
	mtx_actAlm.unlock();
#endif
	// clear active_alarm table
	char *zErrMsg = 0;
	char sql[256];
	cout << "delete active_alarm .\n";
	if (!actAlmDB.exec_sql("delete from active_alarm", zErrMsg)) {
		cout << zErrMsg << endl;
		log(zErrMsg);
		log("delete from active_alarm - error, in updateActiveAlmDB()");
		return false;
	}
	// update to new table
	cout << "insert into active_alarm. \n";
	if (!actAlmDB.beginTransaction())
		return false;
	mtx_actAlm.lock();
	for (const auto & alm : listActAlm) {
		sprintf(sql, "INSERT INTO active_alarm VALUES"
				"('%s','%s','%s')",
				alm.sn.c_str(), alm.semaId.c_str(),alm.devId.c_str());
#if DEBUG_SQL
		cout << "SQL:" << sql << endl;
#endif
		if (!actAlmDB.exec_sql(sql, zErrMsg)) {
			log("insert to database err, in updateActiveAlmDB()");
			actAlmDB.rollback();
			mtx_actAlm.unlock();
			return false;
		}
	}
	mtx_actAlm.unlock();
	if (!actAlmDB.commitTransaction())
		return false;

	mtx_newAlmIn.unlock();

	return true;
}
bool DBHdl::batchMove2ConfirmedDB() {
	char *zErrMsg = 0;
	char sql[255];
	for (const auto & alm : vecAlm) {
		DBFile * almDBF = findTheAlarmDBFile(alm.deviceCode);
		if (almDBF) {
//			if (!almDBF->beginTransaction())
//				return false;
			char ins[256];
			string fsuId = gDat.cfgs[GIDX_FSUID];
			string fsuCode = gDat.cfgs[GIDX_FSUCODE];
			sprintf(ins, "INSERT INTO confirmed_alarm VALUES"
					"('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
					alm.serialNo.c_str(),alm.id.c_str(), fsuId.c_str(), fsuCode.c_str(),
					alm.deviceId.c_str(), alm.deviceCode.c_str(), alm.alarmTime.c_str(),
					alm.alarmLevel.c_str(), alm.alarmFlag.c_str(), alm.alarmDesc.c_str());
#if DEBUG_SQL
			cout << "SQL:" << ins << endl;
#endif
			if (!almDBF->exec_sql(ins, zErrMsg)) {
				log("insert to database err, in saveNewAlarmToConfirmedDB()");
				almDBF->rollback();
				return false;
			}
//			if (!almDBF->commitTransaction())
//				return false;
			sprintf(sql,
					"delete from unconfirm_alarm where sn in (select sn FROM unconfirm_alarm limit %d)",
					vecAlm.size());	//XXX
			if (!almDBF->exec_sql(sql, zErrMsg)) {
#if 1//DEBUG_DB_TABLE
				cout << "batchMove2ConfirmedDB error: " << zErrMsg << endl;
#endif
				log(zErrMsg);
				return false;
			}
#if 1//DEBUG_DB_TABLE
			cout << "***********   batchMove2ConfirmedDBk OK **********" << endl;
#endif
		}
	}
	return true;
}
bool DBHdl::getUnconfirmedAlm() {
	bool rtn = false;
//	vecAlm.clear();
	ClearVector(vecAlm);
	char *zErrMsg = 0;
	for (const auto & dbf : vecAlarmDBs) {
			// ================================= !!!!! get 64 !!!!! =======================
		if (dbf->getTbl("select * FROM unconfirm_alarm limit 64", zErrMsg)) {
			stDBRecord rcd = dbf->getRcd();
			if (rcd.nRow > 0) {
				cout << __FUNCTION__ << "dbf->devId=" << dbf->devId << endl;
//				vecAlm.clear();
				BInt::stAlarm alm;
				for (int r = 0; r < rcd.nRow + 1; ++r) {
					for (int c = 0; c < rcd.nCol; ++c) {
						string s = string(rcd.result[r * rcd.nCol + c]);
#if 1//DEBUG_DB_TABLE
						cout << s << "|";
#endif
						switch (c) {
						case 0:				alm.serialNo = s;			break;
						case 1:
							if (s.size() == 9)
								alm.id = string("0") + s;
							else
								alm.id = s;
							break;
						case 2:				alm.fsuId = s;				break;
						case 3:				alm.fsuCode = s;			break;
						case 4:				alm.deviceId = s;			break;
						case 5:				alm.deviceCode = s;			break;
						case 6:				alm.alarmTime = s;			break;
						case 7:				alm.alarmLevel = s;			break;
						case 8:				alm.alarmFlag = s;			break;
						case 9:				alm.alarmDesc = s;			break;
						}
					}
					cout << endl;
					if (r != 0)		// skip 1st line .
						vecAlm.push_back(alm);
				}
				cout << endl;
				rtn = true;
			}
		} else
			log("select * FROM unconfirm_alarm limit 64 - error, in getUnconfirmedAlm()");
		dbf->clearResult();
	}

	return rtn;
}
void DBHdl::upDateHistoryDat(string devId, string devCode,
		vector<BInt::stSemaphore_l> & vecSem) {

	char *zErrMsg = 0;
	DBFile * historyDBF = findTheHistoryDBFile(devId);
	if (historyDBF) {
		if (!historyDBF->beginTransaction())
			return;
		char sql[256];
		for (const auto & sem : vecSem) {
			sprintf(sql,
					"INSERT INTO history_dat VALUES('%s', '%s', '%s', '%s', '%s', '%s', '%s',"
							" datetime('now','localtime'))", devId.c_str(),
					devCode.c_str(), sem.id.c_str(), sem.type.c_str(),
					sem.measuredVal.c_str(), sem.setupVal.c_str(),
					sem.status.c_str());
//		cout << "sql:" << sql << endl;
			if (!historyDBF->exec_sql(sql, zErrMsg)) {
				log("insert to database err, in upDateHistoryDat()");
				historyDBF->rollback();
				return;
			}
		}
		if (!historyDBF->commitTransaction())
			return;
	}
}
bool DBHdl::getTheDeviceHistorySemaphores(string devId, vector<string> vecIds,
		vector<stSemaphore> & out, string startTime, string endTime) {
	char *zErrMsg = 0;
	char sql[256];
	const char * st = startTime.c_str();
	const char * et = endTime.c_str();

//	out.clear();
	ClearVector(out);
	DBFile * historyDB = findTheHistoryDBFile(devId);
	if (historyDB) {
		for (const auto & sem : vecIds) {
			string fullSemId = gDat.semaIdPreString + sem;
			sprintf(sql,
					"SELECT * FROM history_dat where semId = %s and recordTime between '%s' and '%s'",
					fullSemId.c_str(), st, et);
#if DEBUG_SQL
			cout << "SQL:" << sql << endl;
#endif
			if (historyDB->getTbl(sql, zErrMsg)) {
				stDBRecord rcd = historyDB->getRcd();
//				if (rcd.nRow <= 0)
//					continue;
				for (int r = 0; r < rcd.nRow + 1; ++r) {
					stSemaphore sema;
					for (int c = 0; c < rcd.nCol; ++c) {
#if DEBUG_SQL
						cout << rcd.result[r * rcd.nCol + c] << "|\t";
#endif
						string content(rcd.result[r * rcd.nCol + c]);
						switch (c) {
						case 2:
							if (content.size() == 9)
								sema.id = string("0") + content;
							break;
						case 3:
							sema.type = content;
							break;
						case 4:
							sema.measuredVal = content;
							break;
						case 5:
							sema.setupVal = content;
							break;
						case 6:
							sema.status = content;
							break;
						case 7:
							sema.recordTime = content;
							break;
						}
					}
					if (r != 0) {	// skip title
						out.push_back(sema);
					}
#if DEBUG_SQL
					cout << endl;
#endif
				}
			} else {
	//			log("get history data from database err, in getTheDeviceHistorySemaphores()");
	//			log(zErrMsg);
	//			return false;
				stSemaphore sema(sem, AI_STR); // AI;
				sema.status = "6"; // invalid data
				out.push_back(sema);
			}
			historyDB->clearResult();
		}
	}

	return true;
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#if 0
int test_DB_display_getRcdNum() {
	DBFile db;
	if (!db.init("test.db3"))
		cout << "test.db3 open failed !" << endl;
	cout << "table content:" << endl;
	db.displayTbl("unconfirm_alarm");
	int rtn = 0;
	db.getTblRcdNum("unconfirm_alarm", rtn);
	return rtn;
}
static char dbPath[200];
static sqlite3 *database;
static sqlite3 *openDb() {
	if (sqlite3_open(dbPath, &database) != SQLITE_OK) {
		sqlite3_close(database);
		log(@"Failed to open database: %s", sqlite3_errmsg(database));
	}
	return database;
}
void viewDidLoad() {
	//[super viewDidLoad];
	sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
	log("%d", sqlite3_threadsafe());
	log("%s", sqlite3_libversion());
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	strcpy(dbPath, [[documentsDirectory stringByAppendingPathComponent:@"data.sqlite3"] UTF8String]);
	database = openDb();
	char *errorMsg;
	if (sqlite3_exec(database,
			"CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY AUTOINCREMENT, value INTEGER);",
			NULL, NULL, &errorMsg) != SQLITE_OK) {
		log(@"Failed to create table: %s", errorMsg);
	}
}
static void insertData(sqlite3 * database) {
	char *errorMsg;
	if (sqlite3_exec(database, "BEGIN TRANSACTION", NULL, NULL,
			&errorMsg) != SQLITE_OK) {
		NSLog(@"Failed to begin transaction: %s", errorMsg);
	}
	static const char *insert = "INSERT INTO test VALUES (NULL, ?);";
	sqlite3_stmt *stmt;
	if (sqlite3_prepare_v2(database, insert, -1, &stmt, NULL) == SQLITE_OK) {
		for (int i = 0; i < 1000; ++i) {
			sqlite3_bind_int(stmt, 1, arc4random());
			if (sqlite3_step(stmt) != SQLITE_DONE) {
				--i;
				log(@"Error inserting table: %s", sqlite3_errmsg(database));
			}
			sqlite3_reset(stmt);
		}
		sqlite3_finalize(stmt);
	}
	if (sqlite3_exec(database, "COMMIT TRANSACTION", NULL, NULL,
			&errorMsg) != SQLITE_OK) {
		log(@"Failed to commit transaction: %s", errorMsg);
	}
	static const char *query = "SELECT count(*) FROM test;";
	if (sqlite3_prepare_v2(database, query, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			log(@"Table size: %d", sqlite3_column_int(stmt, 0));
		} else {
			log(@"Failed to read table: %s", sqlite3_errmsg(database));
		}
		sqlite3_finalize(stmt);
	}
}

int DBTest() {
	sqlite3 *db = 0;
	char *zErrMsg = 0;
	int rc;

//打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
	rc = sqlite3_open("test.db", &db);
	if (rc) {
		cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return 0; //exit(1);
	} else
		cout << "You have opened a sqlite3 database named test.db ." << endl;

	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

//查询数据
	/*
	 int sqlite3_get_table(sqlite3*, const char *sql,char***result , int *nrow , int *ncolumn ,char **errmsg );
	 result中是以数组的形式存放你所查询的数据，首先是表名，再是数据。
	 nrow ,ncolumn分别为查询语句返回的结果集的行数，列数，没有查到结果时返回0
	 */
	const char *sql = "SELECT * FROM film ";
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

//int i = 0 ;
	cout << "row:" << nrow << " column= " << ncolumn << endl;
	cout << "The result of querying is : " << endl;

	for (int i = 0; i < (nrow + 1) * ncolumn; i++)
		cout << "azResult[" << i << "] = " << azResult[i] << endl;
	for (int r = 0; r < nrow + 1; ++r) {
		for (int c = 0; c < ncolumn; ++c) {
			cout << azResult[r * ncolumn + c] << "|";
		}
		cout << endl;
	}

	SHOW_TIME("test sqlite3 insert 2000 item start.");

	rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
//执行SQL语句
	char ins[256];
	for (int i = 0; i < 20; ++i) {
		sprintf(ins,
				"INSERT INTO film VALUES('film name %d', %d, %d, 'people %d')",
				i, 1900 + i, i + 2, i);
		rc = sqlite3_exec(db, ins, 0, 0, &zErrMsg);
	}

	rc = sqlite3_exec(db, "COMMIT;", 0, 0, &zErrMsg);
	SHOW_TIME("sqlite3 insert 2000 item end.");

//释放掉  azResult 的内存空间
	sqlite3_free_table(azResult);

	sqlite3_close(db); //关闭数据库
	return 0;

}
int DBTest2() {
	sqlite3 *db = 0;
	char *zErrMsg = 0;
	int rc;

//打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
	rc = sqlite3_open("test2.db", &db);
	if (rc) {
		cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return 0; //exit(1);
	} else
		cout << "You have opened a sqlite3 database named test2.db ." << endl;

	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	const char *sql =
			"CREATE TABLE unconfirmed_alarm ([sn] integer PRIMARY KEY autoincrement,"
					"[id] ,[fsuId] , [deviceId] ,[alarm_time] ,[alarm_leve],[alarm_flag],[alarm_desc])";
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

	rc = sqlite3_exec(db, "BEGIN;", 0, 0, &zErrMsg);
//执行SQL语句
	char ins[256];
	for (int i = 0; i < 3; ++i) {
		sprintf(ins,
				"INSERT INTO unconfirmed_alarm VALUES(NULL, '12345678%02d', '123456789012%02d', '123456789021%02d', "
						"datetime('now'), '二级', '开始', '欠压告警(46.1V)')", i, i + 1,
				i + 2);
		cout << ins << endl;
		rc = sqlite3_exec(db, ins, 0, 0, &zErrMsg);
	}

	rc = sqlite3_exec(db, "COMMIT;", 0, 0, &zErrMsg);
//查询数据
	/*
	 int sqlite3_get_table(sqlite3*, const char *sql,char***result , int *nrow , int *ncolumn ,char **errmsg );
	 result中是以数组的形式存放你所查询的数据，首先是表名，再是数据。
	 nrow ,ncolumn分别为查询语句返回的结果集的行数，列数，没有查到结果时返回0
	 */
	sql = "SELECT * FROM unconfirmed_alarm ";
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

//int i = 0 ;
	cout << "row:" << nrow << " column= " << ncolumn << endl;
	cout << "The result of querying is : " << endl;

//	for(int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
//		cout << "azResult["<< i << "] = " << azResult[i] << endl;
	for (int r = 0; r < nrow + 1; ++r) {
		for (int c = 0; c < ncolumn; ++c) {
			cout << azResult[r * ncolumn + c] << "|";
		}
		cout << endl;
	}

//	SHOW_TIME("test sqlite3 insert 2000 item start.");

//	SHOW_TIME("sqlite3 insert 2000 item end.");

//释放掉  azResult 的内存空间
	sqlite3_free_table(azResult);

	sqlite3_close(db); //关闭数据库
	return 0;

}
#endif

