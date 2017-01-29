//#include "define.h"

#include <unistd.h>
#include <thread>
#include <sstream>
//#include <iostream>
#include <sstream>
#include <string.h>
#include <fcntl.h>
#include "stdio.h"
#include "stdlib.h"
#include <sys/wait.h>

#include "debug.h"
#include "Log.h"
#include "xmlHdl.h"
#include "DB.h"
#include "Message.h"
#include "soap.h"
#include "AppConfig.h"
#include "protocolX.h"
#include "Device.h"
#include "Pipe.h"
#include "ArrayStack.h"
#include "Device.h"
#include "saveDatInTime.h"
//#include "HW.h"
#include "LoginLogout.h"
#include "AutoIncNum.h"
#include "Gatherer.h"
#include "test.h"
#include <execinfo.h>
#include "GetCPUMem.h"
#include "RunStat.h"

using namespace std;
extern DBHdl DB;
extern GlobalDat gDat;
extern DeviceManager DevMgr;
//extern pthread_mutex_t can_writeLock;
AutoIncNum AlmIdx;
LoginHdl loginHdl;
extern Gatherer Gath;
pthread_t tid_mainloop;
RunStat AllRun;

void quit() {
	cout << "################\n";
	cout << "#   Quit...    #\n";
	cout << "################\n";
	AllRun.setRun(false);
	loginHdl.logout();
	FSUServiceStop();
	sleep(3);
//	xmlCleanupParser();
	exit(0);
}
void Stop(int signo) {
	static u32 cnt = 0;
	if (++cnt < 3) {
		cout << "Are you sure ?" << endl;
	} else {
		cout << "OK now ." << endl;
		quit();
	}
}
void Quit(int signo) {
#if DEBUG
	cout << "got quit sig.\n";
#endif
	quit();
}
void checkSEGV(int signo) {
	cout << "mySEGV" << endl;

    void *array[10];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace (array, 20);
    strings = backtrace_symbols (array, size);

    printf ("Obtained %zd stack frames.\n", size);

    for (i = 0; i < size; i++)
            printf ("%s\n", strings[i]);

    free (strings);

 //   exit(0);
}

//pthread_mutex_t fsuInfo_lock = PTHREAD_MUTEX_INITIALIZER;
std::mutex mtx_fsuInfo;
#if 0
void GetMyCPURate(int &) {

	 statics stat;
	 machine_init(&stat);
	 system_info info;
	 get_system_info(&info);
//	 struct top_proc proc;


	 for(;;)
	 {
		 printf("Used CPU:%.1f%%\n",(float)info.cpustates[0]/10);
		 printf("Nice CPU:%.1f%%\n",(float)info.cpustates[1]/10);
		 printf("System CPU:%.1f%%\n",(float)info.cpustates[2]/10);
		 printf("Idle CPU:%.1f%%\n",(float)info.cpustates[3]/10);

		 printf("total memroy:%d\n", info.memory[0]);
		 printf("free memroy:%d\n", info.memory[1]);
		 printf("buffers:%d\n", info.memory[2]);
		 printf("cached:%d\n", info.memory[3]);
		 printf("total swap:%d\n", info.memory[4]);
		 printf("free swap:%d\n", info.memory[5]);

		 sleep(2);
		 printf("..................................\n");
		 get_system_info(&info);

//		 read_one_proc_stat( (pid_t)7443, &proc);

//		 struct top_proc *p = &proc;

//		printf("%s\n",format_next_process(p));
	 }
}
#endif
void *mainLoop(void *) {
	SET_THRD_NAME();
	bool bRebootSet = false;
	unsigned long long now;
	unsigned long long rebootSetTime;
	timeval tp;

//	statics stat;
//	machine_init(&stat);
	system_info info;

	while(AllRun.isRunning()) {
		threadDebug
		string tempCpu, tempMem;
//		tempCpu = floatToString(GetHostCPURate(), 2, 1);		// sleep !! in GetHostCPURate();
//		tempMem = floatToString(getMemOccupyRate(), 2, 1);
//		if (GetMyCPURate(tempCpu) && GetMyMemRate(tempMem)) {
		get_system_info(&info);
//		mtx_fsuInfo.lock();
		gDat.cpuOccRate = floatToString ((float)info.cpustates[0]/10, 2, 1);
		gDat.memOccRate = floatToString ((1-(float)info.memory[1]/(float)info.memory[0])*100, 2, 1);
#if 1 //DEBUG_CPU_MEM
		SHOW_TIME(" # ");
		cout << "CPU:" << gDat.cpuOccRate << "%, ";
		cout << "MEM:" << gDat.memOccRate << "%\n";

//		cout << (float)info.cpustates[0]/10 << endl;
//		cout << info.memory[0] << "," << info.memory[1] << endl;
//		cout << (1-(float)info.memory[1]/(float)info.memory[0])*100 << endl;
#endif
//		mtx_fsuInfo.unlock();

		u32 reboot = getRebootFlag();	// SC send reboot command.
		if (reboot == 1)  {
			gettimeofday(&tp,NULL);
			now  =  tp.tv_sec;// * 1000000 + tp.tv_usec;	// us
			if (!bRebootSet) {
				bRebootSet = true;
				rebootSetTime = now;
				loginHdl.logout();
				sleep(5);
//				notifyPPP("/tmp/fsu_2Dialer_ready2reboot", "1");
				system("/etc/ppp/peers/scripts/vpn_off");
				sleep(2);
				system("reboot");

			} else {
				if ((now - rebootSetTime) > 30)
					system("reboot");
			}
		}
		sleep(3);
	}
	return (void *)0;
}

int main(int argc, char* argv[]) {
//	daemon(1, 1);
	cout << endl << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$                         $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$     FSU program start   $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$                         $$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
	cout << endl << endl << endl;
	log("---- program start... ----");
//	testAll();
	int ch;

    while((ch = getopt(argc,argv,"cxXap")) != -1)
    {
        switch(ch)
        {
        case 'c':
        	gDat.args.en_com = true;
            break;
        case 'x':
        	gDat.args.en_xml = true;
            break;
        case 'X':
			gDat.args.en_xml = true;
			gDat.args.en_xml_cr = true;
			break;
        case 'a':
        	gDat.args.en_alm = true;
            break;
        case 'p':
        	gDat.args.en_parse = true;
            break;
        }
    }

    AllRun.setRun(false);
	signal(SIGINT, Stop);
	signal(SIGQUIT, Quit);
	signal(SIGSEGV, checkSEGV);


	ipaddr(gDat.myIp);
	cout << endl << "My IP : " << gDat.myIp.c_str() << endl << endl;

	if (!AlmIdx.init())
		exit(EXIT_FAILURE);

//	restartGPS(1); // by Hardware

	if (!initConfig())
		goto FAILED;

	SetSCTimeout(3, 3, 3);

	if (!chkInitDat())
		goto FAILED;

	if (!init_productModelTbl())
		goto FAILED;
	if (!init_DevScanTbl())
		goto FAILED;
	if (!init_portCfg()) // 之后才有完整的设备表
		goto FAILED;
	if (!DB.init())			// 不能在前面，因为设备表不全
		goto FAILED;
	if (!DB.getLastActiveAlm())
		goto FAILED;
	if (!init_so_file())
		goto FAILED;

	if(!Gath.init())
		goto FAILED;
	if (!DevMgr.init_local_device())
		goto FAILED;
	if (!DevMgr.initPort())
		goto FAILED;


//	pthread_mutex_init(&can_writeLock,NULL);

	for (auto & devComm : gDat.vecDevComm) {
		devComm->startScan();
	}
	if (!DevMgr.run())
		goto FAILED;

	sleep(3);
	if (!initSaveHistoryDatProc())
		goto FAILED;

	AllRun.setRun(true);

	if (pthread_create(&tid_mainloop, NULL, mainLoop, NULL) < 0) {	// after initConfig(); !!!
		cout << "create main loop thread failed !" << endl;
		goto FAILED;
	}
	if (!loginHdl.loginInit())
		goto FAILED;

	if (!FSUServiceStart())
		goto FAILED;

	notifyPPP("/tmp/fsu_init_stat", "1");
	while(1) {
		;
	}

	cout << "exit success.\n";
	log("exit success\n");
	exit(EXIT_SUCCESS);
	return 0;
FAILED:
	cout << "init failed.\n";
	log("init failed\n");
	notifyPPP("/tmp/fsu_init_stat", "0");

	exit(EXIT_FAILURE);
}
