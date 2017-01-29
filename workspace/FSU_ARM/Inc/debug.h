#ifndef __DEBUG_IF_H__
#define __DEBUG_IF_H__
#include <sys/time.h>

#define _SDEBUG_OUT_ENABLE_            1
#define _SDEBUG_DETAIL_                1
#if _SDEBUG_OUT_ENABLE_ == 1
	extern int printk(const char *fmt, ...);
	#if _SDEBUG_DETAIL_ == 0
		#define PLINE printk("[LJH]<%s:%s:%d> enter_now\n", __FILE__, __func__, __LINE__)
		#define TRACE_LJH(fmt, ...) printk("[LJH]<%s:%s:%d>", __FILE__, __func__, __LINE__, __VA_ARGS__)
	#else
		#define PLINE {\
			static unsigned short cnt = 0; \
			printk("[LJH]<%s:%s:%d:%d> enter_now\n", __FILE__, __func__, __LINE__, ++cnt); }
		#define TRACE_LJH(fmt, ...) { \
			static unsigned short cnt = 0; \
			printk("[LJH]<%s:%s:%d:%d>", __FILE__, __func__, __LINE__, ++cnt, __VA_ARGS__); }
	#endif
#else
	#define PLINE
	#define TRACE_LJH(fmt, ...)
#endif

#define ERROR(...) \
do {\
	fprintf(stderr, "[ERROR  ]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
	fprintf(stderr, __VA_ARGS__); \
}while(0)

#define WARNING(...) \
do {\
	fprintf(stdout, "[WARNING]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
	fprintf(stdout, __VA_ARGS__); \
}while(0)

#define INFO(...) \
do {\
	fprintf(stdout, "[INFO  ]%s %s(Line %d): ",__FILE__,__FUNCTION__,__LINE__); \
	fprintf(stdout, __VA_ARGS__); \
}while(0)

extern unsigned long long gLatestTime;
#define SHOW_TIME_DUR(...) \
do {\
	timeval tp;\
	gettimeofday(&tp, NULL);\
	unsigned long long now = tp.tv_sec*1000000+tp.tv_usec; \
	if(gLatestTime != 0) {	\
		fprintf(stdout, "Used Time: %s[%d], %s: %ld.%ld, %llu ms ", __FILE__, __LINE__, __func__, tp.tv_sec, tp.tv_usec, (now-gLatestTime)/1000);\
		fprintf(stdout, __VA_ARGS__); \
		fprintf(stdout, "\n"); \
	}\
	gLatestTime = now;\
}while(0)
#define SHOW_TIME(...) \
do {\
	timeval t;\
	gettimeofday(&t, NULL);\
	tm * p = localtime(&t.tv_sec);\
	cout << endl;\
	fprintf(stdout, __FUNCTION__); \
	cout << ":";\
	fprintf(stdout, "[%02d:%02d:%02d %06ld] ", p->tm_hour, p->tm_min, p->tm_sec, t.tv_usec);\
	fprintf(stdout, __VA_ARGS__); \
}while(0)
#define step(x)	cout << __FUNCTION__ << ": "<< x << endl

#define DEBUG 1
#define DEBUG_CSV 0
#define DEBUG_INIT_DAT 0
#define DEBUG_INIT_DEVICE 0
#define DEBUG_XML_ID 0
//#define DEBUG_XML_DETAIL 0

//#define DEBUG_COMPORT_DAT 0
#define DEBUG_COMPORT_DAT_SUB_PAK 0
#define DEBUG_COMPORT_HEX_DAT 0

#define DEBUG_PROTOCOL_PARSE_FILE 0
//#define DEBUG_PROTOCOL_PARSE_FILE_DESC 1
#define DEBUG_PACK_PARSE 0
//#define DEBUG_PACK_PARSE_DETAIL 1
#define DEBUG_COMM_IO 0				// comm port data append
#define DEBUG_485_HEART 0
#define DEBUG_SCAN_DEV 0

#define DEBUG_TIME_CHECK
#define DEBUG_TIME_CHECK_A_PORT_IO 0

#define DEBUG_SQL 0
#define DEBUG_DB_TABLE 0

#define DEBUG_ALM_CHK 0
#define DEBUG_ALM_CHK_PROC 1
#define DEBUG_ALM_CHK_END 0
#define DEBUG_ALM_CHK_DETAIL 1
#define DEBUG_ALM_CHK_FILTER 0

#define DEBUG_SEMA_CHK 0
#define DEBUG_DAT_DETAIL 0			// set DI ...
#define DEBUG_DAT_BIT_DAT 0
#define DEBUG_BATT_DAT 0
#define DEBUG_LIGHT_CTRL 0
#define DEBUG_SET_CTRL 0
#define DEBUG_SET_CTRL_CMD_LOCK 0
#define DEBUG_LOCK
#define DEBUG_LOCK_485 0
#define DEBUG_HIS_DAT 0

#define DEBUG_GATHER_ON 1	// 设置为0,无法检测所有19设备的告警，包括主板设备的通信故障!

#define DEBUG_SC_HEART_BREAK 0
#define DEBUG_CPU_MEM 0

#define USE_MAX_SEMA 0

#if DEBUG
	#define DBG(...)\
	do {\
		fprintf(stdout,"[DEBUG]%s %s(Line %d):",__FILE__,__FUNCTION__,__LINE__);\
		fprintf(stdout,__VA_ARGS__);\
	}while(0)
#else
	#define DBG(...)
#endif

#define MLOG(...)\
do {\
	char s[128];\
	sprintf(s,__VA_ARGS__);\
	log(s);\
}while(0)
#define RLock(mtx) \
		std::lock_guard<std::mutex> lock(mtx)
#define WHERE_AM_I 	cout << __FUNCTION__ << endl;

#define threadDebug
/*
	cout << "\t\t\t 国  "; \
	cout << __FUNCTION__ << ":"; \
	printf("thread(%ld)\n", (long int)syscall(224));
*/
#endif

