#include "unistd.h"
#include <string>
#include <mutex>
#include <pthread.h>
#include "scSCServiceSoapBindingProxy.h"
#include "fsuFSUServiceSoapBindingService.h"
#include "sc.nsmap"
#include "fsu.nsmap"
#include "Message.h"
#include "debug.h"
#include "soap.h"
#include "AppConfig.h"
#include "LoginLogout.h"
//const char server[] = "http://192.168.2.51:8080";
extern GlobalDat gDat;
#define fsusvr 1
#define scclt 1
#if fsusvr
// FSU service
using namespace fsu;
unsigned srvBackNbr = 1;
FSUServiceSoapBindingService svr;
#endif
#if scclt
// SC proxy
using namespace sc;
//SCServiceSoapBindingProxy SCLoginProxy;
//SCServiceSoapBindingProxy SCGetterProxy;
SCServiceSoapBindingProxy SCProxy;
#endif
using namespace std;

std::mutex mtx;

extern GlobalDat gDat;
extern LoginHdl loginHdl;
extern RunStat AllRun;
pthread_t tid;

#if scclt
//unsigned nbr = 1;
//void* sayHello_thread(void*) {
//	string s = " hey,你好,hello";
//	string r;
//
//	cout << "start hello loop" << endl;
//	for (int i = 0; i < 1000; ++i) {
//		cout << "FSU --->    : " << s << endl;
//		if (SCProxy.invoke(s, r) == SOAP_OK)
//			std::cout << "    <--- SC:" << r << " (" << nbr++ << ")"
//					<< std::endl;
//		else {
//			cout << "say hello bad end!" << endl;
//			SCProxy.soap_stream_fault(std::cerr);
//		}
//		sleep(1);
//	}
//	return ((void *) 0);
//}
#endif
bool SCLogin_Invoke(string outStr, string & sRtn) {
	bool rtn = false;
	mtx.lock();
	SCProxy.soap_endpoint = gDat.szSCLoginIP;
	if (SCProxy.invoke(outStr, sRtn) != SOAP_OK) {
		DBG("Login invoke error\n");
		SCProxy.soap_stream_fault(std::cerr);
		rtn = false;
	} else
		rtn = true;

	mtx.unlock();
	return rtn;
}
bool SCGetter_Invoke(string outStr, string & sRtn) {
	bool rtn = false;
	mtx.lock();
	SCProxy.soap_endpoint = gDat.szSCGetterIP;
	if (SCProxy.invoke(outStr, sRtn) != SOAP_OK) {
		DBG("Report alarm invoke error\n");
		SCProxy.soap_stream_fault(std::cerr);
		rtn = false;
	} else
		rtn = true;

	mtx.unlock();
	return rtn;
}


void SetSCTimeout(int t1, int t2, int t3) {
	SCProxy.soap->connect_timeout = t1;
	SCProxy.soap->recv_timeout = t2;
	SCProxy.soap->send_timeout = t3;
}
//int soapTest(void) {
//#if scclt
//	pthread_t tid;
//	int ret = pthread_create(&tid, NULL, sayHello_thread, NULL);
//	if (ret != 0) {
//		cout << "create \"say hello\" thread error!" << endl;
//		exit(1);
//	}
//#endif
//#if 0//fsusvr
//	cout << "server running..." << endl;
//	int port = 8080; // server port
//	if (svr.run(port)) {
//		svr.soap_stream_fault(std::cerr);
//		exit(-1);
//	}
//#endif
//#if scclt
//	void *thread_rslt;
//	pthread_join(tid, &thread_rslt);
//#endif
//	return 0;
//}
#if  fsusvr
// as server 
int fsuServiceInvokeResponseTest(string xmlData, string &invokeReturn) {
	const char * in = xmlData.c_str();
	cout << "[" << in << "]" << endl;
	cout << "fsu server rcv :" << xmlData << endl;
	char c[20];
	sprintf(c, "%d", srvBackNbr++);
	invokeReturn = "fsu server back (" + string(c) + "):[" + xmlData + "]";
	return SOAP_OK;

}
int FSUServiceSoapBindingService::invoke(string xmlData, string &invokeReturn) {
	//return fsuServiceInvokeResponseTest(xmlData, invokeReturn);// test
	string rtn;
	if (MessageMgr::inComeMsgHdl(xmlData, rtn))
		invokeReturn = rtn;
	return SOAP_OK;
}
//int FSUServiceStart(int port) {
//	int pt = 8080; // server port
//	if (svr.run(pt)) {
//		svr.soap_stream_fault(std::cerr);
//		exit(-1);
//	}
//	return 1;
//}
extern int NetMode;
void* FSUSvc_thread(void* p) {
	SET_THRD_NAME();
	while(AllRun.isRunning()){
		if ((NetMode == 0) || ((NetMode != 0) && (loginHdl.isVpnOnline()))) {
			threadDebug
			svr.soap->bind_flags = SO_REUSEADDR;
			if (svr.run(gDat.port)) {
				svr.soap_stream_fault(std::cerr);
				cerr << std::cerr << endl;
				sleep(10);
				cout << "try again !" << endl;
				FSUServiceRestart();
			}
		}
	}
	return ((void *) 0);
}
bool FSUServiceStart() {
	int ret = pthread_create(&tid, NULL, FSUSvc_thread, NULL);
	if (ret != 0) {
		cout << "create FSU service thread error!" << endl;
		return false;
	}
	return true;
}

void FSUServiceRestart(){

	svr.reset();
	pthread_cancel(tid);
	sleep(1);
	if (!FSUServiceStart())
		exit(1);
}
void FSUServiceStop() {
	svr.reset();
//	pthread_cancel(tid);
}
#endif

