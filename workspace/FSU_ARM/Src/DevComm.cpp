/*
 * DevComm.cpp
 *
 *  Created on: 2016-7-19
 *      Author: lcz
 */

#include <unistd.h>
#include "define.h"
#include "DevComm.h"
#include <dlfcn.h>
#include <myComm.h>
#include "AppConfig.h"
#include "Gatherer.h"

extern DeviceManager DevMgr;
extern GlobalDat gDat;

unsigned long long gLatestTime = 0;

//std::mutex mtx_can485_wrt;
u8 nBitMask[8] = { 1, 3, 7, 15, 31, 63, 127, 255 };
//bool isCmdOk(const unsigned char *in, unsigned int inlen,
//		const unsigned char * out, unsigned int outLen) {
//	if ((*(in + 5) == *(out + 5)) && (*(in + 6) == *(out + 6))) {
//		if ((*(in + 7) == '0') && (*(in + 8) == '0'))
//			return true;
//	}
//	return false;
//}
ProtocolParse::ProtocolParse(const protocolFormat & fmt) {
	string fn = gDat.exePath + string("lib") + fmt.type + string(".so");
	hDynLib = 0;
#if DEBUG_PROTOCOL_PARSE_FILE
	cout << "create ProtocolParse Obj (" << fn << ")." << endl;
#endif
	hDynLib = dlopen(fn.c_str(), RTLD_LAZY);
	if (hDynLib) {
		if (!LoadPFN()) {
			hDynLib = 0;
			so = string();
		} else {
			so = fn;
			this->fmt = fmt;
		}
	} else {
		cout << "Can't load so file ---> " << fn << endl;
		cout << "Cannot open library: " << dlerror() << '\n';
	}
}
bool ProtocolParse::LoadPFN() {
	char * error;
	LOAD_PFN(getOnePak);
	LOAD_PFN(isCRCOk);
	LOAD_PFN(addCRC);
	LOAD_PFN(getValStr);
	LOAD_PFN(isCmdOk);
	LOAD_PFN(setValStr);
	return true;
}
ProtocolParse::~ProtocolParse() {
#if DEBUG
	cout << "~ProtocolParse() called." << endl;
#endif
	if (hDynLib)
		dlclose(hDynLib);
}
bool ProtocolParse::initProtocolFormat(const vector<string> &cfg) {
	if (cfg.size() < 4)
		return false;
	fmt.soi = cfg[1];
	fmt.eoi = cfg[2];
	fmt.code = cfg[3];
	fmt.type = cfg[0];
	if (fmt.code.size() == 0)
		return false;
	if ((fmt.code != "asc") && (fmt.code != "hex")) {
		cout << "unknown code of protocol." << endl;
		return false;
	}
	return true;
}
string DevComm::bitSetCtrl(const P_SetCtrl & SC, string cmdId, string cmdVal) {

	string out;
	const char * s = cmdId.c_str();
	bool bCtrl = (*(s + 4) == '2');		// DO
	u32 preBits = 0;
	u32 idx = 0;
	u8 lastVal = 0;
	do {
		if (idx >= SC.itms.size())
			break;
		const auto &it = SC.itms.at(idx);
		if ((it.bLen == 0) && (it.BLen == 0)) {
			idx++;
			continue;
		} else if (it.BLen > 0) {
			for (u32 i = 0; i < it.BLen; ++i) {
				cout << "->set fix val str: \"" << it.val << "\""<< endl;
				out = out + it.val;
			}
			idx++;

			continue;
		}
		u8 val = 0;
		string valStr;
		//------------------ get item's val---------------------
		if (it.id.empty()) {
			val = atoi(it.val.c_str());		// predefine data
		} else if (it.id.size() == 3) {
			const char * s = it.id.c_str();
			u32 pos = 0;
			getIdx_xx((char *) s + 1, pos);
			val = prtclDat->L_mem[pos - 1];
#if DEBUG_DAT_BIT_DAT
			cout << "\tidx:"<< (u32)idx << ",L_mem[" << (u32)(pos - 1) << "]=" << (u32)val << endl;
#endif
		} else if (it.id.size() == 8) {
			if ((gDat.semaIdPreString +it.id) == cmdId) {
				if (bCtrl) {
					val = atoi(it.val.c_str());
				} else {
					float f = str2Float(cmdVal);
					val = f;
				}
			} else {
				Device * p = this->pDev;
				float out;
				if (p->getAOValById(/*gDat.semaIdPreString + */it.id, out))
					val = out;
				else
					val = 0;
			}
		} else {
			idx++;
			continue;
		}
		if (it.mul != 1)
			val += it.mul;
		idx++;
		//--------------------------------------------------------
		val = val & nBitMask[it.bLen - 1];
		val = val << (8 - preBits - it.bLen);
		lastVal |= val;
		preBits += it.bLen;
		if (preBits == 8) {
			valStr = (*(currDevDat->ppp->setValStr))(lastVal, 1, 1);
			out = out + valStr;
			cout << "->lastVal(" << (u32)lastVal << ")-valStr\"" << valStr << "\""<< endl;
			preBits = 0;
			lastVal = 0;
		}
	} while (1);

	return out;
}
bool DevComm::loop() {
//	cout << "loop start \n";
	if (!isOnScan()) {
		if (!pDev->isReady())
			return false;
		if (pDev->isCmdIn(cmdId, cmdVal)) {
			onGet = false;
			// comId like '6202001' or 'l01'
	#if DEBUG_SET_CTRL
			cout << "Cmd In : cmdId(" << cmdId << "), cmdVal = \'" << cmdVal << "\'" << endl;
	#endif
			string cmdInf;
			u32 type;
			if (cmdId.size() == 3)
				type = T_SEMA_DO;
			else
				type = getSemaphoreType_fullId(cmdId);
			bool find = false;
			if (T_SEMA_AO == type) {
				for (const auto & i : prtclDat->vecScSet) {	// set
					for (const auto & ii : i.itms) {
						cout << "set itm:" << ii.id << endl;
						string fullId = gDat.semaIdPreString + ii.id;
						if (isSameSemaType(fullId, cmdId)) {
							if (i.itms.size() > 1) {
								cmdInf = i.cmdInf
										+ bitSetCtrl(i, cmdId, cmdVal);
							} else {
								string val = (*(currDevDat->ppp->setValStr))(str2Float(cmdVal),
										ii.BLen, ii.mul);
								string addr;
								getSemaIdx2Str(cmdId, addr);
								if (pDev->isGrpSema(cmdId))
									cmdInf = i.cmdInf + ii. val + addr + val;
								else
									cmdInf = i.cmdInf + ii.val + val;
							}
							find = true;
							break;
						}
					}
					if (find)
						break;
				}
			} else if (T_SEMA_DO == type) {
				bool bOnOffCtrl = false;
				if (cmdId.size() != 3) {// 标准8位id
					for (const auto & i : prtclDat->vecScCtrl) {	// ctrl
						for (const auto & ii : i.itms) {
							if (isSameSemaType(gDat.semaIdPreString + ii.id, cmdId)) {
								if (ii.val.size() > 0) {
									bOnOffCtrl = true;
									for (const auto & j : prtclDat->vecSame) {	// ctrl
										if (j.id == ii.id) {
											string statId = j.dxx;	//开关状态
											u32 semaIdx_1 = getSemaphoreIdx(statId);
											u32 semaIdx2_0 = getSemaphoreIdx2(statId) - 1;
											u8 out = 0;
											pDev->getDIStatById(semaIdx_1, semaIdx2_0, out);
											const char * s = ii.val.c_str();
											u8 v = *s - '0';
											if (v == out) {
												cmdInf = i.cmdInf;
												find = true;
											}
										}
									}
									if (!find) {
										bOnOffCtrl = false;
									}
								}
							}
						}
					}
				}
				/*
				if (!bOnOffCtrl) {
					cout << "size is " << prtclDat->vecScCtrl.size() << endl;
					for (const auto & i : prtclDat->vecScCtrl) {	// ctrl
						for (const auto & ii : i.itms) {
							cout << "ctrl itm:" << ii.id << endl;
							string fullId;
							if (ii.id.size() == 0)
								continue;
							if (ii.id.size() != 3)	// 标准8位id
								fullId = gDat.semaIdPreString + ii.id;
							else
								fullId = ii.id;
							if (isSameSemaType(fullId, cmdId)) {
								if (i.itms.size() > 1) {
									cmdInf = i.cmdInf
											+ bitSetCtrl(i, cmdId, string());
								} else {
									if (pDev->isGrpSema(cmdId)) {
										string addr;
										getSemaIdx2Str(cmdId, addr);
										cmdInf = i.cmdInf + ii.val + addr;
									} else {
										cmdInf = i.cmdInf + ii.val;
									}
								}
								find = true;
								break;
							}
						}
						if (find)
							break;
					}
				}*/
				if (!bOnOffCtrl) {
					cout << "size is " << prtclDat->vecScCtrl.size() << endl;
					for (const auto & i : prtclDat->vecScCtrl) {	// ctrl
						for (const auto & ii : i.itms) {
							cout << "ctrl itm:" << ii.id << endl;
							if (ii.id.size() == 0)
								continue;
							bool verify = false;
							if (ii.id.size() != 3) {	// 标准8位id
								string fullId = gDat.semaIdPreString + ii.id;
								if (isSameSemaType(fullId, cmdId))
									verify = true;
							} else {
								if (ii.id == cmdId)
									verify = true;
							}
							if (verify) {
								if (i.itms.size() > 1) {
									cmdInf = i.cmdInf
											+ bitSetCtrl(i, cmdId, string());
								} else {
									if (pDev->isGrpSema(cmdId)) {
										string addr;
										getSemaIdx2Str(cmdId, addr);
										cmdInf = i.cmdInf + ii.val + addr;
									} else {
										cmdInf = i.cmdInf + ii.val;
									}
								}
								find = true;
								break;
							}
						}
						if (find)
							break;
					}
				}
			}
			if (find) {
	#if 1//DEBUG_SET_CTRL
				cout << "cmdInf: " << cmdInf << endl;
	#endif
				makeCmdPak(cmdInf, oBuf, outLen);
				output(oBuf, outLen);
				return true;
			} else {
				pDev->setSetPointResult(cmdId, STAT_SETPOINT_INVALID_DAT);	// unused cmd id !!
				return false;
			}
		} else {
	//		SHOW_TIME("duty\n");
			cntDuty++;
			u32 interval;
//			if(scanable) {
				interval = currDevDat->defCfg->maxdelay / 100;
//			} else {

//			}
			bool bInTrap = (commTrapCounter >= COMM_TRAP_ERR_MAX_NUM);
			if (bInTrap) {
	//			cout << "\t ** got it !\n";
				interval *= 2;
			}
			if (cntDuty >= interval) {
				if (bInTrap)
					commTrapCounter = 0;
				cntDuty = 0;
				onGet = true;
//				if (pDev->)
				if (!prtclDat->vecGet.empty()) {
					makeDutyReqPak(oBuf, outLen);
					output(oBuf, outLen);
					nextGet();
					return true;
				}
			}
		}
	} else {
		cntDutyScan++;
		if (cntDutyScan >= 1) {	// 20 * 100 ms, 2s 扫描一个设备
			cntDutyScan = 0;
			u32 sIdx;
			mtx_scanIdx.lock();
			scl
			sIdx = scanIdx;
			scu
			mtx_scanIdx.unlock();
			u32 realIdx = sIdx;
			if (scanMode == 1) {	// 按排序过的顺序发送
				realIdx = sortedTblOfMode1[sIdx];
			}
			currDevDat = & (scanTbl.at(realIdx));
			if (!initPort()) {
				return false;
			}
			if (scanTbl.size() > 0) {
				makeScanPak(oBuf, outLen);
				output(oBuf, outLen);
				nextGet();
				return true;
			}
		}
	}

	return false;
}
void DevComm::nextGet() {
	if (isOnScan()) {
		mtx_scanIdx.lock();
		scl
#if DEBUG_SCAN_DEV
		cout << "** NextScan - " << scanIdx << endl;
#endif
		if (scanMode1_devConn && (scanIdx == 0))
			scanMode1_devConn_1Loop = true;
		scanIdx = (scanIdx + 1) % scanTbl.size();
		scu
		mtx_scanIdx.unlock();

	}else{
//		cout << "** NextGet - " << getIdx << endl;
		getIdx = (getIdx + 1) % prtclDat->vecGet.size();
	}
}
void * DevCommDutyProc(void * param) {
	SET_THRD_NAME();
	DevComm * dc = (DevComm *) param;
	while (1) {
//		dc->setInDutyStat(true);
		if (dc->loop()) { // maybe false,
//			threadDebug
			// calculate timeout
//			struct timespec abstime;
//			getAbsTime(abstime, 1000);	//  STD is 500
//			pthread_mutex_lock(&dc->mtx_cond);
//			int ret = pthread_cond_timedwait(&dc->cond_rcv, &dc->mtx_cond, &abstime);
//			pthread_mutex_unlock(&dc->mtx_cond);
			std::unique_lock < std::mutex > lk(dc->mtx_cond_rcv);
			std::cv_status ret = dc->cv.wait_for(lk, std::chrono::milliseconds(2000));
#if DEBUG_SCAN_DEV
			u32 realIdx;
			dc->mtx_scanIdx.lock();
			scl
			if (dc->scanMode == 1) {	// 按排序过的顺序发送
				realIdx = dc->sortedTblOfMode1[dc->scanIdx];
			} else
				realIdx = dc->scanIdx;
			scu
			dc->mtx_scanIdx.unlock();
#endif
			if (ret == std::cv_status::timeout) {	// timeout
#if DEBUG_TIME_CHECK_A_PORT_IO
				SHOW_TIME(" # ");
#endif
#if DEBUG
				if (dc->isOnScan()) {
#if DEBUG_SCAN_DEV
					cout << " scan " << dc->ttyType << dc->ttyPort << ", idx = " << realIdx;
					cout  << " <---- timeout\n";
#endif
				} else
					cout << " Dev(" << dc->pDev->fullId << ") <---- timeout\n";
#endif
				dc->clear();
				dc->noPackResponseHdl();
				dc->commEvent(false);	// timeout
				sleep(1);
			} else {
				if (dc->isOnScan()) {
#if DEBUG_SCAN_DEV
					cout << " scan " << dc->ttyType << dc->ttyPort << ", idx = " << realIdx;
					cout  << " <---- received data.\n";
#endif
				}
				dc->commEvent(true);	// rcved
			}
		}
//		dc->setInDutyStat(false);
		usleep(100000);		// 100 ms, 对应Loop()里的计数器(maxdelay/10)
		if (!dc->isRunning()) {
			cout << "quit duty thread.\n";
			break;
		}
	}
	return ((void *) 0);
}
void * inPackCheckProc(void * param) {
	SET_THRD_NAME();
	DevComm * dc = (DevComm *) param;
//	unsigned long long now;
//	timeval tp;
	while (1) {
//		dc->setPackChkSta(true);
		while (dc->isOnRcv()) {
//			threadDebug
			if (dc->comPort) {
				if(!dc->comPort->me(dc))
					continue;
			} else {
//				dc->setPackChkSta(false);
				continue;
			}
//			cout << "[*] device-" << dc << endl;
			auto now = system_clock::now();
			auto dur = duration_cast < milliseconds > (now - dc->m_last);
			int dif = dur.count();
			if (!dc->comPort->isEmpty()) {
//				cout << "inPackCheckProc(). isNewPackIn?\n";
				if (dc->comPort->isNewPackIn(dc->m_lastPos)) {	// new pack
					dc->mtx_inbuf.lock();
					dc->m_last = now;
					if (!dc->m_gotHead) {
						dc->m_gotHead = true;
					}
					dc->mtx_inbuf.unlock();
//					if (dc->packHdl(dc->comPort->m_buf,	dc->comPort->pos())) {
//						cout << " 1.*** wait time = " << dif << " ms" <<endl;
//					}
				} else {
					if ((dif > 200) && (dif < 500)) {
#if DEBUG_COMPORT_DAT_SUB_PAK
//						cout << "try..." << endl;
#endif
						//bool rslt =
						dc->packHdl();
#if DEBUG_COMPORT_DAT_SUB_PAK
						if (rslt) {
							cout << "  wait time = " << dif << " ms" <<endl;
						} else {
							cout << "*";
						}
#endif
					}
					else if (dif > 500) {
						dc->badPackResponseHdl();
					}
				}
			}
			usleep(20000);// was 20
		}
//		dc->setPackChkSta(false);
		if (!dc->isRunning()) {
			cout << "quit pack check thread.\n";
			break;
		}

	}
	return ((void *) 0);
}
u32 DevComm::output(unsigned char * pBuf, u32 len) {
//	pthread_mutex_lock(&mutex_inbuf_lock);
	mtx_inbuf.lock();
	m_last = system_clock::now();
	m_lastPos = 0;
	m_gotHead = false;
	mtx_inbuf.unlock();
//	pthread_mutex_unlock(&mutex_inbuf_lock);
	if (comPort->connBy485())
		comPort->setFD(m_subFD);
	setOnRcv(true);
#if DEBUG_TIME_CHECK_A_PORT_IO
	SHOW_TIME(" # ");
#endif
//#if DEBUG_COMPORT_DAT
	if (gDat.args.en_com) {
		// 扫描阶段，设备还没确定，不能输出信息！
		cout << " Dev(" << pDev->fullId << ")" << " ----> ";
		if (currDevDat->ppp->fmt.code == "asc")
			printAsc(pBuf, len);
		else
			printHex(pBuf, len);
	}
//#endif
	u32 rtn = comPort->writeSerial(pBuf, len);
#if DEBUG_TIME_CHECK_A_PORT_IO
//	SHOW_TIME(" # end \n");
#endif

	return rtn;
}
void DevComm::commEvent(bool commOk) {
	if (commOk)
		commErrCounter = 0;
	else {
		if (commErrCounter < COMM_ERR_MAX_NUM)
			++commErrCounter;
		else {
			if (scanable) {
				setOnScan(true);
				pDev->setReady(false);
			}
		}
	}
	RLock(mtx_comm);
	bCommErr = commErrCounter >= COMM_ERR_MAX_NUM;
}
bool DevComm::isCommErr() {
	bool rtn = false;
	do {
		RLock(mtx_comm);
		rtn = bCommErr;
	} while (0);
	return rtn;
}
bool DevComm::isOnRcv() {
	bool rtn = false;
	mtx_onRcv.lock();
	rtn = m_onRcv;
	mtx_onRcv.unlock();
	return rtn;
}
void DevComm::setOnRcv(bool b) {
	RLock(mtx_onRcv);
//	cout << " ***** set owner ! ***  " << this << endl;
	comPort->setOwner(this);
	m_onRcv = b;
}
bool DevComm::isOnScan() {
	bool rtn = false;
	mtx_onScan.lock();
	rtn = onScan;
	mtx_onScan.unlock();
	return rtn;
}
void DevComm::setOnScan(bool b) {
	RLock(mtx_onScan);
	onScan = b;
}
DevComm::DevComm() {
}
DevComm::~DevComm() {
#if DEBUG
	cout << "~DevComm() called." << endl;
	cout << "quit devId is " << pDev->fullId << endl;
#endif
	quitThreadProc();
	if (tid_packChk != 0) {
		pthread_join(tid_packChk, NULL);
		tid_packChk = 0;
	}
	if (tid_duty != 0) {
		pthread_join(tid_duty, NULL);
		tid_duty = 0;
	}
	if (m_buf) {
		delete[] m_buf;
		m_buf = NULL;
	}
	if (sortedTblOfMode1) {
		delete [] sortedTblOfMode1;
		sortedTblOfMode1 = NULL;
	}
	if (comPort) {
		delete comPort;
		comPort = NULL;
	}
}
bool DevComm::parseThePak(unsigned char * const buf, u32 size) {
	u32 remain;
	bool ok = false;
	if ((comPort->connBy485()) || (nTTyType == 1)){ //byzuo
		u32 temp = 0;
		//my_transform 已经把包拼好
		if ((*(currDevDat->ppp->getOnePak))(buf, size, m_buf, temp, remain))
			ok = true;
	} else {
//		cout << "before,size=" << m_bufPos << " - ";
//		m_buf[m_bufPos] = 0;
//		printAsc(m_buf, m_bufPos);
		if ((*(currDevDat->ppp->getOnePak))(buf, size, m_buf, m_bufPos, remain)) {
//		if (getOnePak(buf, size, m_buf, m_bufPos, remain)) {
			ok = true;
		}
//		cout << "after,size=" << m_bufPos << " - ";
//		m_buf[m_bufPos] = 0;
//		printAsc(m_buf, m_bufPos);
	}

	if (ok) {
//		cout << "ok::";
//		printHex(m_buf, m_bufPos);
		if ((*(currDevDat->ppp->isCRCOk))(m_buf, m_bufPos)) {
//		if (isCRCOk(m_buf, pakLen)) {
//			cout << "pak ok, ready to parse.\n";
#if DEBUG_TIME_CHECK_A_PORT_IO
				SHOW_TIME(" # ");
#endif
//#if DEBUG_COMPORT_DAT
		if (gDat.args.en_com) {
			u32 sIdx;
			mtx_scanIdx.lock();
			scl
			sIdx = scanIdx;
			scu
			mtx_scanIdx.unlock();
			if (currDevDat->ppp->fmt.code == "asc") {
				if (isOnScan()) {
					cout << " scan dev(" << sIdx << ") <---- size( " << m_bufPos << "):";	// << c << endl;
				} else
					cout << " Dev(" << pDev->fullId << ") <---- size( " << m_bufPos << "):";	// << c << endl;
#if DEBUG_COMPORT_DAT_SUB_PAK
				printHex(m_buf, m_bufPos);
#endif
				m_buf[m_bufPos] = 0;
				printAsc(m_buf, m_bufPos);
			} else {
				if (isOnScan())
					cout << " scan dev(" << sIdx << ") <---- size( " << m_bufPos << "):";
				else
					cout << " Dev(" << pDev->fullId << ") <---- size( " << m_bufPos << "):";
				for (u32 i = 0; i < m_bufPos; ++i)
					printf("%02X.", *(m_buf + i));
				cout << endl;
			}
		}
//#endif
			u32 last = 0;
			if (isOnScan()) {
				mtx_scanIdx.lock();
				scl
				last = lastScanIdx;
				scu
				mtx_scanIdx.unlock();
				if (last < scanTbl.size()) {
//					for (u32 i = 0; i < outLen; ++i)
//						printf("%02X@", *(oBuf + i));
//					cout << endl;
					 if ((*(currDevDat->ppp->isCmdOk))(m_buf, m_bufPos, oBuf, outLen)) {
						 parseScanRtnPak(m_buf, m_bufPos);
					 } else {
						 cout << "scan pak check error !\n";
					 }
				}
			} else {
				if (onGet) {
					if (last < prtclDat->vecGet.size()) {
						 if ((*(currDevDat->ppp->isCmdOk))(m_buf, m_bufPos, oBuf, outLen)) {
	//					 if (isCmdOk(m_buf, m_bufPos, oBuf, outLen)) {
							 parseThePak2(m_buf, m_bufPos);
							 commTrapCounter = 0;
						 } else {
							 commTrapCounter++;
							 cout << "\t ** got it got it \n";
						 }
					}
				} else {	// Set or Ctrl
	#if DEBUG_SET_CTRL
				cout << " ########## Set or Ctrl back !! #################" << endl;
	#endif
					u32 result = ((*(currDevDat->ppp->isCmdOk))(m_buf, m_bufPos, oBuf, outLen)) ?
							STAT_SETPOINT_OK : STAT_SETPOINT_DEV_RTN_FAIL;
					pDev->setSetPointResult(cmdId, result);
				}
			}
			clear();
			cv.notify_one();
			return true;
		} else {
			if (!onGet) {
				pDev->setSetPointResult(cmdId, STAT_SETPOINT_INVALID_DAT);
			}
			cout << "\t !!!!! bad crc ,clear\n";
			comPort->clear();
			clear();
			cv.notify_one();
		}
	}

	return false;
}
void DevComm::parseScanRtnPak(unsigned char * const buf, u32 size) {
	bool find = false;
	u32 last = 0;
	mtx_scanIdx.lock();
	scl
	last = lastScanIdx;
	scu
	mtx_scanIdx.unlock();

	if (scanMode == 1) {// 循环2次，有优先级
#if DEBUG_SCAN_DEV
		cout << "***********scan rcv data, len = " << size << "*************\n";
		if (gDat.args.en_com)
			cout << buf << endl;
#endif
		u32 realIdx = last;
		if (scanMode == 1) 	// 按排序过的顺序发送
			realIdx = sortedTblOfMode1[last];
		const devIdentifyItm & it = scanInitTbl->identifyTbl[realIdx];
		if ((it.way % 10) == 0) {
			find = true;
		} else {
			mtx_scanIdx.lock();
			scl
			if (!scanMode1_devConn)
				scanMode1_devConn = true;
			else {
				if (scanMode1_devConn_1Loop) {
					find = true;
					scanMode1_devConn = false;
					scanMode1_devConn_1Loop = false;
				}
			}
			scu
			mtx_scanIdx.unlock();
		}
	} else {			// 收到，根据返回内容直接判断，内容为空时参考返回数据长度
#if DEBUG_SCAN_DEV
		cout << "*********** scan rcv data, len = " << size << "*************\n";
		if (gDat.args.en_com)
			cout << buf << endl;
#endif
		const devIdentifyItm & it = scanInitTbl->identifyTbl[last];
		u32 sz = it.ids.size();
		if (sz == 0) {
			if (it.rtnLen != 0) {
				if (it.rtnLen == size)
					find = true;
			} else
				find = true;
		} else {
			string s((const char*)buf);
			u32 got = 0;
			for (const auto & i : it.ids) {
				if (i.first.empty()) {
					got++;
					continue;
				} else {
					if (i.first == "00/01") {
						if (s.find("00", i.second) != (u32)-1)
							got++;
						else if (s.find("01", i.second) != (u32)-1)
							got++;
					} else {
						if (s.find(i.first, i.second) != (u32)-1)
							got++;
					}
				}
			}
			if (got == sz)
				find = true;
		}
	}
	if (find) {
#if DEBUG_SCAN_DEV
		cout << "scan and find devie at [" << last << "]\n";
#endif
		setOnScan(false);
		mtx_scanIdx.lock();
		scl
		scanIdx = 0;
		scu
		mtx_scanIdx.unlock();
		u32 realIdx = last;
		if (scanMode == 1) 	// 按排序过的顺序发送
			realIdx = sortedTblOfMode1[last];
		prepareDevice(realIdx);

	}
}
void DevComm::setPrtcl() {
	for (auto & i : gDat.prtclDatTbl) {
		string fileName = findFileNameOfPath(i->devName);
#if DEBUG_SCAN_DEV
		cout << "looking ... " << fileName << endl;
#endif
		if (fileName == currDevDat->defCfg->productModel_name) {
			prtclDat = i;
			break;
		}
	}
}
void DevComm::prepareDevice(u32 idxInScanTbl) {
	currDevDat = &scanTbl.at(idxInScanTbl);
#if DEBUG
	cout << "Prepare Device (" << currDevDat->defCfg->productModel_name << ")\n";
#endif
	setPrtcl();

	string lastModel = modelIdx;
	modelIdx = scanTbl.at(idxInScanTbl).defCfg->productModelIdx;	// new
//	step(1);
	if(lastModel != modelIdx) {
		if (!lastModel.empty()) {
			pDev->clearAlarm();
			pDev->setReady(false);
#if DEBUG_INIT_DEVICE
			cout << "clearAlarm done !\n";
#endif
			pDev->clearMem();
//			sleep(2);
		}
//		step(21);
#if DEBUG_INIT_DEVICE
		cout << "ready to change device !\n";
#endif
		DevMgr.changeTheDevice(pDev,
				gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType +  strDevId,
				modelIdx);
//		step(23);
	} else
		pDev->setReady(true);
}
void DevComm::parseThePak2(unsigned char * const buf, u32 size) {
//	cout << "\t****" <<  __FUNCTION__ << ": 0\n";
	vector<stIdVal> dat;
#if DEBUG_COMPORT_HEX_DAT
	cout << "parse dat - size(" << size << "):";
	for (u32 i = 0; i < size; ++i)
		printf(" %02X", buf[i]);
	cout << endl;
#endif
	const P_Get & getItm = prtclDat->vecGet.at(lastGetIdx);
	u32 pos = 0;
	s32 idx = 0;
	s32 lBracketIdx = -1;
	s32 loops = 0;		// 循环次数
	u32 semaIdx2_0 = 0;	// 循环下标
	u32 lastVal = 0;	// for loops;
	u32 val = 0;
	bool bInLoop = false;
	bool bLastIsByte = true;
	s32 idx_bitStart = 0;
	s32 idx_bitEnd = 0;
	s32 idx_copy = 0;
	do {
//		cout << "\t****" <<  __FUNCTION__ << ": 1\n";
#if DEBUG_PACK_PARSE
		cout << " * pos = " << pos << endl;
#endif
		if ((idx < 0) || (idx >= (s32) getItm.itms.size()))
			break;
		const auto &getIt = getItm.itms.at(idx);
		if (getIt.type == T_GRP_BRACKET_RIGTH) {		// " } "
			semaIdx2_0++;
			if (--loops > 0) {
#if DEBUG_PACK_PARSE
				cout << "  >";
#endif
				idx = lBracketIdx;	// back to " { "
			} else {
#if DEBUG_PACK_PARSE
				cout << "** }" << endl;
#endif
				bInLoop = false;
				semaIdx2_0 = 0;
				idx++;
			}
			continue;
		} else {
			if (getIt.type == T_GRP_BRACKET_LEFT) {	// " { "
				if (!bInLoop) {		// first in
					loops = lastVal;
					lBracketIdx = idx;
					bInLoop = true;
					semaIdx2_0 = 0;
#if DEBUG_PACK_PARSE
					cout << "** { loops = " << loops << ", back Idx = " << idx << endl << "  <";
#endif
				} else {
#if DEBUG_PACK_PARSE
					cout <<" <" << endl;
#endif
				}
			}
		}
		if (getIt.BLen > 0) {
			if ((!bInLoop || (loops > 0)) && (getIt.id.size() != 0)) {
//				const unsigned char * const start = buf + pos;
				string valStr;
#if 1//DEBUG_PACK_PARSE_DETAIL
				string strId;
				if (getIt.id.size() == 8)
					strId = semaIdCalcAdd(getIt.id, semaIdx2_0);
				else
					strId = getIt.id;
#endif
				const char * c = getIt.id.c_str();
#if DEBUG_PACK_PARSE
				cout << "id(" << strId.c_str() << ")";
#endif
				if (*c == 'd') {	// DI dxx
					valStr = (*(currDevDat->ppp->getValStr))(buf + pos, getIt.BLen, 1, getIt.bSign);
//					if (semaIdx2_0 == 3)
//						valStr = "229";
//#if DEBUG_PACK_PARSE_DETAIL
if (gDat.args.en_parse) {
					cout << "   dxx = " << getIt.id;
#if DEBUG_COMPORT_HEX_DAT
					cout << ", hex:";
					printHexNo_n(buf+pos, getIt.BLen);
#endif
					cout << ", val = " << valStr << endl;
}
//#endif
					bool bFind = false;
					if (!valStr.empty()) {	// not empty
						u32 dVal = atoi(valStr.c_str());
						if (dVal == 0) {	// alarm data is 0
							for (const auto & sameItm : prtclDat->vecSame) {
								if (getIt.id == sameItm.dxx) {
									string strId = semaIdCalcAdd(sameItm.id, semaIdx2_0);
									dat.push_back(stIdVal(strId, DI::sDisact));
									bFind = true;
								}
							}
						} else {	// alarm data is not 0
							for (const auto & sameItm : prtclDat->vecSame) {
								if (getIt.id == sameItm.dxx) {
									const char *s = sameItm.val.c_str();
									u32 sVal = 0;
									if (sameItm.val.size() == 1)
										sVal = Char2Num(*s);	// ex: 0, 1, ...
									else
										sVal = Ch2_2_Num(*s, *(s + 1));	// ex: e1,e5,e9,...
									string strId = semaIdCalcAdd(sameItm.id, semaIdx2_0);
									bool equ = (dVal == sVal);
									string & val = equ ? DI::sAct :  DI::sDisact;
									if (equ)
										bFind = true;
									dat.push_back(stIdVal(strId, val));
								}
							}
						}
						if (!bFind) {
							cout<< "\n###############################################################################"<< endl;
							cout  << "####################    undefined DI value of dxx !!     ######################"<< endl;
							cout  << "###############################################################################\n"							<< endl;
						}
					}
				} else if (*c == 'n') { //nxx for grp data size
	//						cout << "\ngetValStr start" << endl;
					valStr = (*(currDevDat->ppp->getValStr))(buf + pos, getIt.BLen, 1, getIt.bSign);
	//						cout << "getValStr end" << endl;
//	#if DEBUG_PACK_PARSE_DETAIL
					if (gDat.args.en_parse)
						cout << "   ** nxx=" << valStr << endl;
//	#endif
					val = atoi(valStr.c_str());
					if (val > 30) {
						cout << "pos=" << pos << endl;
						printHex(buf, size);
						cout << "###############################################################################" << endl;
						cout << "###############################################################################" << endl;
						cout << "###############################################################################" << endl;
						cout << "###############################################################################" << endl;
						cout << "################# ############### ###################      ####################" << endl;
						cout << "################# # ############# ################ ######### ##################" << endl;
						cout << "################# ### ########### ############## ############# ################" << endl;
						cout << "################# ##### ######### ############## ############# ################" << endl;
						cout << "################# ####### ####### ############## ############# ################" << endl;
						cout << "################# ######### ##### ############## ############# ################" << endl;
						cout << "################# ########### ### ############## ############ #################" << endl;
						cout << "################# ############# # ################ ######### ##################" << endl;
						cout << "################# ############### ##################       ####################" << endl;
						cout << "###############################################################################" << endl;
						cout << "###############################################################################" << endl;
						cout << "###############################################################################" << endl;
					} else {
						lastVal = val;
						if (pDev)
							pDev->updateNxxGrpSize(getIt.id, val);
					}
				} else if ((*c == 'b')) {
					if (currDevDat->ppp->fmt.code == "asc") {
						if (getIt.BLen == 2) {	// 2 char 转 1 byte
							char h = *(buf + pos);
							char l = *(buf + pos + 1);
							ascStrOneBitHdl(getIt.id, h, l);
						}
					} else {
						if (getIt.BLen == 1) {	// 2 char 转 1 byte
							char c = *(buf + pos);
							char h = (c >> 4) & 0x0f;
							h = (h > 9) ? (h -10 + 'A') : (h + '0');
							char l = c & 0x0f;
							l = (l > 9) ? (l -10 + 'A') : (l + '0');
							ascStrOneBitHdl(getIt.id, h, l);
						}
					}
				} else {
					valStr = (*(currDevDat->ppp->getValStr))(buf + pos, getIt.BLen, getIt.mul, getIt.bSign);
//#if DEBUG_PACK_PARSE_DETAIL
					if (gDat.args.en_parse) {
#if DEBUG_COMPORT_HEX_DAT
						cout << " Hex:";
						printHexNo_n(buf+pos, getIt.BLen);
#endif
						cout << "|\t" << valStr << "\t";
					}
//#if DEBUG_PROTOCOL_PARSE_FILE_DESC
//#if DEBUG_PACK_PARSE_DETAIL
					if (gDat.args.en_parse) {
						cout << "(" << getIt._desc << ")";
						cout << endl;
					}
//#endif
					val = atoi(valStr.c_str());
					if (pDev) {
						if (pDev->isGrpSizeId(getIt.id)) { // normal id of grp data size
							if (val > 30) {
								cout << "pos=" << pos << endl;
								printHex(buf, size);
								cout << "###############################################################################" << endl;
								cout << "###############################################################################" << endl;
								cout << "###############################################################################" << endl;
								cout << "###############################################################################" << endl;
								cout << "################# ############### ###################      ####################" << endl;
								cout << "################# # ############# ################ ######### ##################" << endl;
								cout << "################# ### ########### ############## ############# ################" << endl;
								cout << "################# ##### ######### ############## ############# ################" << endl;
								cout << "################# ####### ####### ############## ############# ################" << endl;
								cout << "################# ######### ##### ############## ############# ################" << endl;
								cout << "################# ########### ### ############## ############ #################" << endl;
								cout << "################# ############# # ################ ######### ##################" << endl;
								cout << "################# ############### ##################       ####################" << endl;
								cout << "###############################################################################" << endl;
								cout << "###############################################################################" << endl;
								cout << "###############################################################################" << endl;
							} else {
								lastVal = val;
//	#if DEBUG_PACK_PARSE_DETAIL
//								cout << "group size (" << getIt.id << ") is " << val << endl;
//	#endif
								pDev->updateGrpSize(getIt.id, val);
							}
						}
						string strId = semaIdCalcAdd(getIt.id, semaIdx2_0);
						dat.push_back(stIdVal(strId, valStr));
	#if DEBUG_DAT_DETAIL
	//							cout << "\t\t\t++ push dat --> id(" << strId << "), val = (" << valStr << ")" << endl;
	#endif
					}
				}
			}
			if (bInLoop) {
				if (loops > 0)
					pos += getIt.BLen;
			} else {
				pos += getIt.BLen;
			}
			idx++;
			bLastIsByte = true;
		}
		else if (getIt.bLen > 0) {	// 转位处理
			if (bLastIsByte) {
				idx_bitStart = idx;
				idx_copy = idx;
				s32 sz = getItm.itms.size();
				while(1) {
					idx_copy++;
					if (idx_copy >= sz) {
						idx_bitEnd = idx_copy;
						break;
					}
					auto & it = getItm.itms.at(idx_copy);
//					cout << "it.id=:" << it.id << "," << it.bLen << "," << it.BLen << "," << it.type << endl;
					if ((it.BLen > 0) || (it.type == T_GRP_BRACKET_RIGTH)) {
						idx_bitEnd = idx_copy;
						break; // bit items end;
					}
				};
				u32 Bytes = parseThePak_bit(buf + pos, idx_bitStart, idx_bitEnd);
				cout << "parseThePak_bit end" << endl;
				idx = idx_bitEnd;
				pos += Bytes;
			}
			bLastIsByte = false;
		} else {
			if ((bInLoop && (loops == 0)) || (getIt.id.size() == 0)) {
				pos += getIt.BLen;
				idx++;
			}
		}
	} while (1);
//	cout << "\t****" <<  __FUNCTION__ << ": 2\n";
	pDev->upDateSemaphoes(dat);
//	cout << "\t****" <<  __FUNCTION__ << ": 3\n";
}
void DevComm::ascStrOneBitHdl(string bxx, char h, char l) {
	vector<stIdVal> dat;
	for (const auto &i : prtclDat->vecBit) {
		if (i.bxx != bxx)
			continue;
		u8 c = Ch2_2_Num(h, l);
		u8 val = 0;
		u32 fromBit = 0;
		for (const auto &j : i.itms) {
			if (!j.first.empty()) {
				if (j.first.size() == 8) {	// id
					val = c >> (8 - fromBit - j.second);
					val &= nBitMask[j.second - 1];
//					if (getIt.mul != 1)
//						val -= getIt.mul;
					string valStr;
					char buf[32];
					snprintf(buf, sizeof(buf), "%d", val); // snprintf is thread safe #include <stdio.h>
					valStr.append(buf);
//#if DEBUG_PACK_PARSE_DETAIL
					if (gDat.args.en_parse) {
						cout << "id(" << j.first << "), bitVal=" << valStr << endl;
					}
//#endif
					dat.push_back(stIdVal(j.first, valStr));
				}
			}
			fromBit += j.second;
		}
	}
	pDev->upDateSemaphoes(dat);
}
u32 DevComm::parseThePak_bit(unsigned char * const buf, s32 start, s32 end) {
	vector<stIdVal> dat;

	const P_Get & getItm = prtclDat->vecGet.at(lastGetIdx);
	u32 val = 0;
	s32 idx = start;
	u32 byteIdx = 0;
	u32 fromBit = 0;
	string valStr;

	for (idx = start; idx < end; ++idx) {
		const auto & getIt = getItm.itms.at(idx);
#if DEBUG_DAT_BIT_DAT
		cout << "byteIdx=" << byteIdx << endl;
#endif
		if (getIt.bLen == 0) {
			continue;
		}
		if (getIt.id.empty()) {
			fromBit += getIt.bLen;
			if (0 == (fromBit % 8)) {
				byteIdx += (fromBit / 8);
				fromBit = 0;
			}
		} else {	// id
#if DEBUG_DAT_BIT_DAT
			cout << "fromBit:" << fromBit << ",bits=" << getIt.bLen << endl;
#endif
			char c = *(buf + byteIdx);
//			printf("c = %02x\n", c);
			c = c >> (8 - fromBit - getIt.bLen);
			c &= nBitMask[getIt.bLen - 1];
			val = c;

			if (getIt.id.size() == 3) {
				char * s = (char *) getIt.id.c_str();
				u32 idx = 0;
				getIdx_xx(s + 1, idx);
				prtclDat->L_mem[idx - 1] = val;
#if DEBUG_DAT_BIT_DAT
				cout << "set L_mem[" << idx  - 1<< "]=" << val << endl;
#endif
			} else if (getIt.id.size() == 8) {
				if (getIt.mul != 1)
					val -= getIt.mul;
				char buf[32];
				snprintf(buf, sizeof(buf), "%d", val); // snprintf is thread safe #include <stdio.h>
				valStr.append(buf);
				dat.push_back(stIdVal(getIt.id, valStr));
#if DEBUG_DAT_DETAIL
				cout << "id[" << getIt.id << "] valStr=\"" << valStr << "\"" << endl;
#endif
			}
			fromBit += getIt.bLen;
			if (0 == (fromBit % 8)) {
				byteIdx += (fromBit / 8);
				fromBit = 0;
			}
		}
	};
	pDev->upDateSemaphoes(dat);
	return byteIdx;
}
void DevComm::noPackResponseHdl() {
	if (!onGet)
		pDev->setSetPointResult(cmdId, STAT_SETPOINT_NO_REPONSE);

}
void DevComm::badPackResponseHdl() {
	if (!onGet)
		pDev->setSetPointResult(cmdId, STAT_SETPOINT_BAD_REPONSE);
	cout << "bad pack response, clear.\n";
	clear();
	cv.notify_one();

}
bool DevComm::packHdl() {
	unsigned char tmp[4096];
	unsigned tmpLen = 0;
	comPort->getBufCopy(tmp, tmpLen);
//		cout << "----before trans:";
//		printHex(tmp, tmpLen);
	if ((comPort->connBy485()) || (comPort->getTTY() == 1)) { //byzuo
//		printHex(m_buf, m_bufPos);
		if (my_transform(m_subFD, comPort->getTTY(), m_buf, &m_bufPos, tmp, tmpLen) == 0) {
//			cout << "----after trans 0:";
//			printHex(m_buf, m_bufPos);
			return false;
		} else {
//			cout << "----after trans 1:";
//			printHex(m_buf, m_bufPos);
			return parseThePak(m_buf, m_bufPos);
		}
	}
	return parseThePak(tmp, tmpLen);
}
void DevComm::makePak(string szIn, unsigned char * szOut, size_t &len) {
	const char *in = (char *) szIn.c_str();
	size_t size = szIn.size();
//	cout << __FUNCTION__ << "1\n";
	if (currDevDat->ppp->fmt.code == "hex") {
		if ((size % 2) == 1) {
#if DEBUG
			cout << "bad format for hex string !" << endl;
#endif
			*szOut = 0;
			len = 0;
		}
		for (u32 i = 0; i < (size / 2); ++i) {
			szOut[i] = Ch2_2_Num(in[i * 2], in[i * 2 + 1]);
		}
		szOut[size / 2] = 0;
		len = size / 2;
#if DEBUG
//	const char * ss = szOut;
//	for (u32 i = 0; i < size/2; ++i) {
//		printf("%02x ", *(ss + i));
//	}
//	printf("\n");
#endif

	} else if (currDevDat->ppp->fmt.code == "asc") {
		for (u32 i = 0; i < size; ++i)
			szOut[i] = in[i];
		len = size;
	} else {
#if DEBUG
		cout << "unknown code of protocol." << endl;
#endif
		*szOut = 0;
		len = 0;
	}
//	cout << __FUNCTION__ << "2\n";
	(*(currDevDat->ppp->addCRC))(szOut, len);  // len handled here!
//	if (fmt.code == "hex") {
//		len += 2;
//	} else if (fmt.code == "asc") {
//		len += 5;
//	}
//	cout << __FUNCTION__ << "3\n";
}
void DevComm::makeScanPak(unsigned char * szOut, u32 &len) {
	mtx_scanIdx.lock();
	scl
	lastScanIdx = scanIdx;
	u32 realIdx = scanIdx;
	if (scanMode == 1) {	// 按排序过的顺序发送
		realIdx = sortedTblOfMode1[scanIdx];
	}
	scu
	mtx_scanIdx.unlock();
	string s = pakHead + scanInitTbl->identifyTbl.at(realIdx).outStr;
	makePak(s, szOut, len);
}
void DevComm::makeDutyReqPak(unsigned char * szOut, u32 &len) {
	lastGetIdx = getIdx;
	if (prtclDat->vecGet.size() <= 0)
		return;
	string inStr;
//	if (fmt.type == "tcl") {
//		inStr = string("00") + cfg.addr + vecGet.at(getIdx).cmdInf;
//	} else {
		inStr = pakHead + prtclDat->vecGet.at(getIdx).cmdInf;
//	}
#if DEBUG_PACK_PARSE
	cout << "Dev[" << pDev->id << "] getIdx=" << getIdx << endl;
#endif
	makePak(inStr, szOut, len);
}
void DevComm::makeCmdPak(string & cmdInf, unsigned char * szOut, u32 &len) {
	string inStr;
//	if (fmt.type == "tcl") {
//		inStr = string("01") + cfg.addr + cmdInf;
//	} else {
		inStr = pakHead + cmdInf;
//	}
	makePak(inStr, szOut, len);
}
void DevComm::setScanMode() {
	u32 sz = scanInitTbl->identifyTbl.size();
	for (const auto & scanDev : scanInitTbl->identifyTbl) {
		if ((scanDev.way % 10) != 0) {
			scanMode = 1;
			break;
		}
	}
	if (scanMode == 1) {
		u32 tmp[sz];
		for (u32 i = 0 ; i < sz ; ++i)
			tmp[i] = scanInitTbl->identifyTbl.at(i).way;
		cout << endl;
		InsertSort(tmp, sz);
		cout << endl;

		sortedTblOfMode1 = new u32[sz];
		for (u32 i = 0 ; i < sz ; ++i) {
			for (u32 j = 0 ; j < sz ; ++j) {
//				cout << tmp[i] << "," << scanInitTbl->identifyTbl.at(j).way << endl;
				if (tmp[i] == scanInitTbl->identifyTbl.at(j).way) {
					sortedTblOfMode1[i] = j;
				}
			}
		}
//		for (u32 i = 0 ; i < sz ; ++i)
//			cout << sortedTblOfMode1[i] << ",";
//		cout << endl;
	}
	scanSize = scanInitTbl->identifyTbl.size();
}
bool DevComm::initScanTbl(const portScanTblItm & in) {
	onScan = true;
	scanInitTbl = &in;
	devType = scanInitTbl->devType;
#if DEBUG_INIT_DAT
	cout << "identify table size is " << scanInitTbl->identifyTbl.size() << endl;
#endif
	for (const auto & d : scanInitTbl->identifyTbl) {
		bool findModel = false;
		bool findPPP = false;
		for(const auto & model : gDat.productDefCfgInitTbl) {
			if (d.devModel == atoi(model.productModelIdx.c_str())) {
				findModel = true;
				scanDat sd;
				sd.defCfg = &model;
				string name = model.productModel_name;
#if DEBUG_INIT_DAT
				cout << name << "\t";
#endif
				for (const auto & pp : gDat.vecPPP) {
					if (pp->fmt.type == model.format) {
#if DEBUG_INIT_DAT
						cout << pp->so << endl;
#endif
//						cout << pp->fmt.type << "," << model.format << endl;
						findPPP = true;
						sd.ppp = pp;
						scanTbl.push_back(sd);
						break;
					}
				}
				if (!findPPP)
					break;
			}
		}
		if (!findModel || !findPPP) {
			cout << "scan table init error !" << endl;
			return false;
		}
	}
	setScanMode();
	return true;
}
// 扫描确定设备后才设置
//void DevComm::initDefCfg(const stDevDefCfgItm & c) {
//	defCfg = &c;
//}
extern Gatherer Gath;
void DevComm::startScan() {
//	for (auto & devComm : gDat.vecDevComm) {
//		for (const auto & i : devComm->scanTbl)
//			cout << i.defCfg->ver << "," << i.defCfg->addr << endl;
//	}
	if (scanTbl.size() == 0)
		setOnScan(false);


	// 扫描确定设备后
//	initDefCfg(sss);
//	if (!init2())
//		cout << "Device(" << dev->pDev->id << " init2() error !" << endl;

}
bool DevComm::initPortDat() {
	if (ttyType == string("ttyE")) {
		nTTyType = *(gDat.cfgs[GIDX_FSUCOMMTYPE].c_str()) - '0';
		bExDev = true;
		comPathName = ttyPort;
		int port = atoi(ttyPort.c_str());
		m_subFD = port / 10 * 16 + port % 10;
		if (nTTyType == 2) {						// 485
			comPort = Gath.comPort485;
			b485 = true;
		} else if  (nTTyType == 1) { //byzuo		// can
			comPort = new MultiThrdSerial;
		} else {
			cout << "wrong FSU Comm type !\n";
			return false;
		}
	}else if (ttyType == string("ttyS")) {			// main
		comPort = new MultiThrdSerial;
		comPathName = string("/dev/") + ttyType + ttyPort;
	} else {
		cout << "wrong portType (" << ttyType<< ") in port.conf file !\n";
		return false;
	}

	return true;
}
bool DevComm::initWorkThread() {
	runStat.setRun(true);
	if (pthread_create(&tid_duty, NULL, DevCommDutyProc, this) < 0)
		return false;
	if (pthread_create(&tid_packChk, NULL, inPackCheckProc, this) < 0)
		return false;

	return true;
}
bool DevComm::setDevice(string devId) {
	// find Device , and set it's DevComm obj.
//	Device * p = DevMgr.findDevice(gDat.cfgs[GIDX_SITE_CODE] + gDat.siteType + cfg.Id);
	Device * p = DevMgr.findDevice(devId);
	if (p) {
		p->setCommPort(this);
		pDev = p;
		return true;
	} else
		return false;
}
bool DevComm::init(const string & tty, const string & port, const string & did) {
	ttyType = tty;
	ttyPort = port;
	strDevId = did;
	modelIdx = "";
	if (!initPortDat())
		return false;

	return true;
}
void DevComm::init_buf_head() {
	if (m_buf != NULL) {
		delete [] m_buf;
		m_buf = NULL;
	}
	m_buf = new u8[currDevDat->defCfg->bufSize];
	pakHead = currDevDat->ppp->fmt.soi + currDevDat->defCfg->ver + currDevDat->defCfg->addr;
#if DEBUG_PROTOCOL_PARSE_FILE
	cout << "head = " << pakHead << endl;
#endif
}
bool DevComm::initPort() {
//	if (!setDevice(sDevice_Full_Id) )
//		return false;
	init_buf_head();
	if (comPort->connBy485())
		comPort->initSetFD(m_subFD);
	if (bExDev) {
		OpenCommPort();
	} else {
		comPort->closePort();
		if (!OpenCommPort()){
			return false;
		}
	}

	return true;
}
bool DevComm::configPort() {
#if DEBUG_SCAN_DEV
	cout << "[ ";
	cout << currDevDat->defCfg->baudrate << ",";
	cout << currDevDat->defCfg->parity << ",";
	cout << (u32)currDevDat->defCfg->databit << ",";
	cout << currDevDat->defCfg->stopbit << " ] ";
	cout << currDevDat->defCfg->bufSize << ", ";
	cout << (u32) bExDev << endl;
#endif
	return comPort->open_config(comPathName.c_str(), nTTyType,
			currDevDat->defCfg->baudrate,
			currDevDat->defCfg->databit,
			currDevDat->defCfg->stopbit.c_str(),
			currDevDat->defCfg->parity,
			currDevDat->defCfg->bufSize,
			bExDev);
}
bool DevComm::OpenCommPort() {
	if (!configPort()) {
		comPort->closePort();
		cout << "open comm port - " << ttyType << ttyPort << " error!"
				<< endl;
		return false;
	}
	return true;
}

