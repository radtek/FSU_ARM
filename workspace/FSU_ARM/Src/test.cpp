/*
 * test.cpp
 *
 *  Created on: 2016年10月2日
 *      Author: lcz
 */
#include <unistd.h>
#include <thread>
#include <map>
#include <vector>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include "stdio.h"
#include "stdlib.h"

#include "define.h"
#include "Log.h"
#include "Pipe.h"
#include "SerialPort.h"
#include "MultiThrdSerial.h"
#include "HFAlarm.h"
#include "test.h"
using namespace std;

#if TEST
class A {
public:
	~A() {
		cout << "delete A\n";
		delete [] m;
	}
	void set(int i) {
		m = new int[i];
	}
private:
	int * m = NULL;
};
class B {
public:
	~B() {
		cout << "delete B\n";
		delete [] m;
	}
	void set(int i) {
		m = new A[i];
	}
	void setA(int idx, int i) {
		m[idx].set(i);
	}

private:
	A * m = NULL;
};
void testC11() {
	map<string, vector<int> > map;
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	map["one"] = v;

	for (const auto & it : map) {
		cout << it.first << endl;
		for (auto v : it.second)
			cout << v << endl;
	}
	int cc[3] = { 1, 2, 3 };
	for (int & it : cc)
		cout << it * it << endl;

	struct st {
		st(string _a, string _c, bool _b) {
			a = _a;
			c = _c;
			b = _b;
		}
		string a;
		string c;
		bool b;
	};
	vector<st> sts;
	sts.push_back(st(string("1"), string("2"), true));
	sts.push_back(st(string("3"), string("4"), false));
	sts.push_back(st(string("5"), string("2"), true));
	sts.push_back(st(string("7"), string("8"), false));
	sts.push_back(st(string("9"), string("0"), true));

	string s("5");
	const st * pst;
	for (const auto & cfg : sts) {
		if (s == cfg.a) {
			pst = &cfg;
			break;
		}
	}
	cout << pst->a << "," << pst->c << endl;

}
void test() {
//	SHOW_TIME("start time calc.");
//	xmlParseTest();
//	SHOW_TIME("xmlParseTest() end");
//	double db = 123.456;
//	cout << "val is " << floatToString(db, 3, 1) << endl;
//	cout << "val is " << floatToString(db, 2, 4) << endl;
//	cout << "val is " << floatToString(db, 5, 1) << endl;
//	cout << "val is " << floatToString(db, 3, 6) << endl;
//	cout << "val is " << floatToString(db, 0, 2) << endl;
//	cout << "val is " << floatToString(db, -1, 3) << endl;
//	string s = string("");
//	string s;
//	cout << "size = " << atoi(s.c_str()) << endl;

//	testPk();

//	testC11();
//	SHOW_TIME("testC11 end");

//	DBTest();

// sc client test.
#if 0
	B* b = new B;
	b->set(3);
	for (int i = 0; i < 3; ++i)
		b->setA(i, 4);
	delete b;
	ArrayStack<int, 3> stk;

	stk.push(1);
	stk.push(2);
	stk.push(3);
	stk.push(4);
	int out;
	if (stk.pop(out))
	cout << out << endl;
	if (stk.pop(out))
	cout << out << endl;
	if (stk.pop(out))
	cout << out << endl;
	if (stk.pop(out))
	cout << out << endl;
	if (stk.pop(out))
	cout << out << endl;

	FixSizeFIFO <int,6> dat;

	dat.addNew(1);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(1);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(2);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(3);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(4);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(5);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(6);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(7);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(8);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(9);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(10);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(11);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(12);
	cout << dat.first() << "," << dat.end() << endl;
	dat.addNew(13);
	cout << dat.first() << "," << dat.end() << endl;

#endif
#if 0
	string in = "中文";
	string out;
	SCInvoke(in, out);
	cout << "SCInvoke test:" << endl << out << endl;

//	soapTest();
#endif
	//	char *s = ".a06a12.34err";
	//	float f = atof(s);
	//	string valStr = floatToString(f,0,0);
	//	string valStr1 = floatToString(f,0,1);
	//	string valStr2 = floatToString(f,0,2);
	//	string valStr3 = floatToString(f,0,3);
	//	string valStr4 = floatToString(f,1,0);
	//	string valStr5 = floatToString(f,2,0);
	//	string valStr6 = floatToString(f,3,0);
	//	string valStr7 = floatToString(f,3,3);
	//	string valStr8 = floatToString(f,4,4);
	//
	//	cout << valStr << ",";
	//	cout << valStr1 << ",";
	//	cout << valStr2 << ",";
	//	cout << valStr3 << ",";
	//	cout << valStr4 << ",";
	//	cout << valStr5 << ",";
	//	cout << valStr6 << ",";
	//	cout << valStr7 << ",";
	//	cout << valStr8 << ",";
	//	cout << endl;
	//
#if 0
	char c[2] = {('0' + 2), 0};
	string s = string(c);
	cout << s << endl;
	const char * cc = "100h111";
	int i = atoi(cc);
	cout << i << endl;
	string s2 = string(cc);
	cout << s2 << endl;

	const char *ff = "21.10c23.s";
	float f = atof(ff);
	cout << f << endl;

	//	string s1 = string(NULL);
	string s2 = string();
	string s3 = string("");
	//	cout << "s1.size() = " << s1.size() << endl;
	cout << "s2.size() = " << s2.size() << endl;
	cout << "s3.size() = " << s3.size() << endl;
	//	cout << "s1 = (" << s1 << ")" << endl;
	cout << "s2 = (" << s2 << ")" << endl;
	cout << "s3 = (" << s3 << ")" << endl;

#endif
	while (0) {
		log("test");
		sleep(30);
	}
	u32 * u;
	u = new u32;
	*u = 222;
	*(u + 3) = 4444;
	cout << "*u = " << *u << endl;
	delete[] u;
	cout << "*u = " << *u << endl;

//	string s("001234500");
	string s("-1grd");
	int i = atof(s.c_str());
	cout << "i = " << i << endl;
	i += 2;
	stringstream newStr;
	newStr << i;

	string ss = newStr.str();

	cout << "new val is " << ss << endl;

}
void error_fatal(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "strerror() is %s\n", strerror(errno));
	exit(1);
}
int fd1, fd2;
void* com1_read_thread(void*) {
	int nread;
	char buff[256];
	while (1) {
		while ((nread = read(fd1, buff, 256)) > 0) {
//			buff[nread+1] = '\0';
//			printf( "\n%s", buff);
			for (int i = 0; i < nread; ++i) {
				printf("%02x ", buff[i]);
			}
			printf("\n");
			usleep(20);
		}
	}
	return ((void *) 0);
}
void* com2_read_thread(void*) {
	int nread;
	char buff[256];
	while (1) {
		while ((nread = read(fd2, buff, 256)) > 0) {
//			buff[nread+1] = '\0';
//			printf( "\n%s", buff);
			for (int i = 0; i < nread; ++i) {
				printf("%02x ", buff[i]);
			}
			printf("\n");
			usleep(20);
		}

	}
	return ((void *) 0);
}
void* com2_write_thread(void*) {
	string s = " COM2: hello ";
	int nwrite = 0;
	for (int i = 0; i < 400; ++i) {
		int len = s.size();
		nwrite = write(fd2, (char*) s.c_str(), len);
		if (nwrite != len)
			printf("send error\n");
		usleep(10000);
	}
	return ((void *) 0);
}
void* com1_write_thread(void*) {
	string s = " COM1: hello ";
	int nwrite = 0;
	for (int i = 0; i < 300; ++i) {
		int len = s.size();
		nwrite = write(fd1, (char*) s.c_str(), len);
		if (nwrite != len)
			printf("send error\n");
		usleep(10000);
	}
	return ((void *) 0);
}
pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
pthread_t tid4;
void comTest1() {
//	int count = 0;
//	char status[10];
	//打开/dev/ttyS0 串口
	fd1 = OpenComPort((char *) "/dev/ttyS2", 115200, 8, "1", 'N');
	if (fd1 == -1) {
		printf("open com port 1 error!\n");
		exit(1);
	}
//	fd2 = OpenComPort("/dev/ttyUSB1", 115200, 8, "1", 'N');
//	if (fd2 == -1) {
//		printf("open com port 2 error!\n");
//		exit(1);
//	}
//	int ret;
//	ret = pthread_create(&tid1, NULL, com1_read_thread, NULL);
//	if (ret != 0) {
//		cout << "create com1_read_thread error!" << endl;
//		exit(1);
//	}
//	ret = pthread_create(&tid2, NULL, com2_read_thread, NULL);
//	if (ret != 0) {
//		cout << "create com2_read_thread error!" << endl;
//		exit(1);
//	}
//	ret = pthread_create(&tid3, NULL, com1_write_thread, NULL);
//	if (ret != 0) {
//		cout << "create com1_write_thread error!" << endl;
//		exit(1);
//	}
//	ret = pthread_create(&tid4, NULL, com2_write_thread, NULL);
//	if (ret != 0) {
//		cout << "create com2_write_thread error!" << endl;
//		exit(1);
//	}
}
MultiThrdSerial com1;
MultiThrdSerial com2;
void comTest2() {
//	if (!com1.init("/dev/ttyUSB0")) {
//		printf("open com1 port init error!\n");
//		return;
//	}
//	if (!com1.config(115200, 8, "1", 'N')) {
//		printf("open com1 port config error!\n");
//		return;
//	}
//	if (!com1.serialOpen()) {
//		printf("open com1 port read thread create error!\n");
//		return;
//	}
//	if (!com1.openPort("/dev/ttyS2", true, 115200, 8, "1", 'N', 4096)) {
//		com1.closePort();
//		printf("open com port 1 error!\n");
//		return;
//	}
//	if (!com2.openPort("/dev/ttys1", true, 115200, 8, "1", 'N')) {
//		com2.closePort();
//		printf("open com port 2 error!\n");
//		return;
//	}

	sleep(3);
#if 0
	string s1 = " 11111  COM1: hello ";
//	string s2 = " 22222  COM2: hello ";
	int len1 = s1.size();
//	int len2 = s2.size();
	int nwrite1 = 0;
//	int nwrite2 = 0;
	for (int i = 0; i < 400; ++i) {
		nwrite1 = com1.writeSerial((unsigned char*) s1.c_str(), len1);
		if (nwrite1 != len1)
			printf("send 1 error\n");
		usleep(1000000);
//		nwrite2 = com2.writeSerial((char*) s2.c_str(), len2);
//		if (nwrite2 != len2)
//			printf("send 2 error\n");
//		usleep(100000);

	}
#endif
}
void tests() {
//	string s= "";
//	char c= *(s.c_str());
//	int i = c;
//	printf("%d\n", i);
	while (1) {
		notifyCamera();
		sleep(2);
	}
}
void HighFreqAlarmTest() {
	HFAlarm alm;

	for (int i = 0; i < 7; ++i) {
		time_t t = time(NULL);
		if (alm.addNewAndCheckActive(t)) {
			cout << "active !!!" << endl;
		} else
			cout << "not active ." << endl;
		sleep(2);
	}
	for (int i = 0; i < 5; ++i) {
		if (alm.isDisactive()) {
			cout << "isDeactive() ? = not active ." << endl;
		} else
			cout << "isDeactive() ? = active !!!" << endl;
		sleep(3);
	}
}

pthread_cond_t mycond;
pthread_mutex_t mymutex;
int mydata;
void *myfun1(void *) {

	u32 timeout_ms = 2000;
	while (1) {
		int ret;

		struct timespec abstime;
		struct timeval now;
		gettimeofday(&now, NULL);
		int nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
		abstime.tv_nsec = nsec % 1000000000;
		abstime.tv_sec = now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;

		pthread_mutex_lock(&mymutex);
		// pthread_cond_wait(&mycond, &mymutex);
		ret = pthread_cond_timedwait(&mycond, &mymutex,	&abstime);

		if (ret != 0) {
			printf("myfun1 timeout in thread(%d)\n ", (int)pthread_self());
			pthread_mutex_unlock(&mymutex);
			continue;
		}

		while (mydata) {
			printf("consume in %d (mydata = %d)\n ", (int)pthread_self(), mydata);
			mydata--;
		}
		pthread_mutex_unlock(&mymutex);
	}
	return (void *)0;
}
void *myfun2(void *) {
	while (1) {
		pthread_mutex_lock(&mymutex);
		printf("produce in %d\n ", (int)pthread_self());
		mydata++;
		pthread_mutex_unlock(&mymutex);
		pthread_cond_signal(&mycond);
		sleep(3);
	}
	return (void *)0;
}
int condWaitTest() {
	mydata = 0;
	pthread_t mythread1, mythread2;

	pthread_cond_init(&mycond, NULL);
	pthread_mutex_init(&mymutex, NULL);

	pthread_create(&mythread1, NULL, myfun1, NULL);
	pthread_create(&mythread2, NULL, myfun2, NULL);
// pthread_create(&mythread3,NULL,myfun1,NULL);
// pthread_create(&mythread3,NULL,myfun2,NULL);

	pthread_join(mythread1, NULL);
	pthread_join(mythread2, NULL);
// pthread_join(mythread3,NULL);

	return 0;
}
void *longProc(void *) {
	u32 a = 1;
	for (u32 i = 0; i < 100000000; ++i) {
		if ((i % 2) == 0)
			a = a + i;
		else
			a += a * i;
		if ((i % 10000000) == 1)
			cout << "test : " << i << endl;
	}
	return (void *)0;
}
//void consttest(char * const p) {
//	char c = 'c';
//	p = &c;
//	*p = 2;
//}
#if 0
string getValStr2(const unsigned char *s, unsigned int datLen, int mul)
{
	if (mul < 0) { // 同时表示是浮点数和浮点的小数为数
		if (datLen != 4) {
			return string();
		} else {
			typedef	union {
				unsigned int		Word;				            //字操作定义
				struct 	{
					unsigned int	M 	            :23;		    //
					unsigned int	E 		    	:8;		        //
					unsigned int	S 	    		:1;		        //

				}Bits;
				struct {
					unsigned int	Byte1 	        :8;		         //	low
					unsigned int	Byte2 	        :8;		         //
					unsigned int	Byte3 	        :8;		         //
					unsigned int	Byte4 	        :8;		         //	high
				}Bits_8;
			}uDataFlow;
			uDataFlow Dataflow;
			double tmp;

			Dataflow.Bits_8.Byte1 = *(s + 3);
			Dataflow.Bits_8.Byte2 = *(s + 2);
			Dataflow.Bits_8.Byte3 = *(s + 1);
			Dataflow.Bits_8.Byte4 = *s;
			tmp=Dataflow.Bits.M;

			tmp=(0x00000001<<(Dataflow.Bits.E-127))
					*(1+tmp/0x00800000);
			if (Dataflow.Bits.S)
				tmp = -tmp;
			return floatToString(tmp, 0, 0 - mul);
		}
	} else {
		unsigned int val;
		switch (datLen) {
			case 2:
				val = (s[0] << 8) + s[1];
				break;
			case 4:
				val = (s[0] << 24) + (s[1] << 16) + (s[2] << 8) + s[3];
				break;
			default:
				return string();
		}
		char buf[32];

		string out;
		if (mul == 1) {
			char buf[32];
			snprintf (buf, sizeof (buf), "%d", val);  // snprintf is thread safe #include <stdio.h>
			out.append (buf);
		} else {
			double tmp = val;
			tmp /= mul;
			unsigned int a;
			if (mul > 10000)
				a = 5;
			else if (mul > 1000)
				a = 4;
			else if (mul > 100)
				a = 3;
			else if (mul > 10)
				a = 2;
			else if (mul > 1)
				a = 1;
			else
				a = 0;
			out = floatToString(tmp, 0, a);
		}
		out.append (buf);
		return out;

	}
}
void floatStrTest() {
	float f1 = str2Float(string("53.46"));
	cout << "f val is " << f1 << endl;
	string val =  setValStr(f1, 4, 2);
	cout << "f str is " << val << endl;
	float f2 = 342.345;
	unsigned char *s = (unsigned char *)&f2;
	char ch[9] = {0};
	unsigned char cch[5] = {0};
	for (u32 i = 0; i < 4; ++i) {
		char c = *(s + i);
		char h = (c >> 4) & 0x0f;
		char l = c & 0x0f;
		ch[i * 2    ] = (h >= 10) ? (h - 10 + 'A') : (h + '0');
		ch[i * 2 + 1] = (l >= 10) ? (l - 10 + 'A') : (l + '0');
		cch[i] = c;
	}
	cout << "val is " << ch << endl;
	float* ff = (float*)&cch[0];
	float fff = *ff;
	printf("float is %3.2f", fff);
}
#endif
char chx(char c) {
	if (c >= 10)
		return c - 10 + 'A';
	else
		return c + '0';
}
//void floatTest() {
//	float f = 342.345;
//	unsigned char *s = (unsigned char *)&f;
//	char ch[9] = {0};
//	unsigned char cch[5] = {0};
//	for (u32 i = 0; i < 4; ++i) {
//		char c = *(s + i);
//		char h = (c >> 4) & 0x0f;
//		char l = c & 0x0f;
//		ch[i * 2    ] = (h >= 10) ? (h - 10 + 'A') : (h + '0');
//		ch[i * 2 + 1] = (l >= 10) ? (l - 10 + 'A') : (l + '0');
//		cch[i] = c;
//	}
//	cout << "val is " << ch << endl;
//	float* ff = (float*)&cch[0];
//	float fff = *ff;
//	printf("float is %3.2f", fff);
//}
void test_remove_if() {
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
	list <actAlmItm> lst;
	lst.push_back(actAlmItm("001", "sema01","dev01"));
	lst.push_back(actAlmItm("002", "sema02","dev01"));
	lst.push_back(actAlmItm("003", "sema03","dev01"));
	lst.push_back(actAlmItm("004", "sema01","dev01"));
	lst.push_back(actAlmItm("005", "sema01","dev01"));
	cout << "before--\n";
	for (auto & it : lst)
		cout << "|"<< it.sn << "|" << it.devId << "|" << it.semaId << "|\n";
	lst.remove_if([&](const actAlmItm & i) {
						return (i.sn == string("003"));
					});
	cout << "after--\n";
	for (auto & it : lst)
		cout << "|"<< it.sn << "|" << it.devId << "|" << it.semaId << "|\n";

}
void testAll() {
//	int n = test_DB_display_getRcdNum();
//	tests();
//	cout << "record num is " << n << endl;
//	DBTest();
//	DBTest2();
//	HighFreqAlarmTest();
//	comTest2();
//	dlLoadTest();

	test_remove_if();
//	condWaitTest();
	pthread_t myThrd;

	pthread_create(&myThrd, NULL, longProc, NULL);

	cout << "waiting..." << endl;
	pthread_join(myThrd, NULL);
	cout << "thread end ." << endl;

	// write csv file test.
//	char path[256];
//	string p = get_exe_path(path, 256) + string("cfg/init_DI.csv");
//	if (writeCfgToCsv(p, "6001001", 4, "24" ))
//		cout << "writeCfgToCsv() OK !" << endl;
//	else
//		cout << "writeCfgToCsv() Error !" << endl;

//	string asdf = string();
//	float f = str2Float(asdf);
//	cout << "f=" << f << endl;
//	string ssss("1");
//	char *ss = "1";
////	int c = strtoull(ss, NULL, 10);		// good.
//	u8 c8 = strtoull(ss, NULL, 10);	// wrong !!!!!!!!!
//	cout << "val = " << dec << c << endl;
	//setConfig(GIDX_FSUIP, string("12.34.5.6"), false);
	//	DevMgr.exit();	// just for test
	string s = "红外sdf门磁算算呵呵算";
	int rslt = s.find("呵呵", 0);
	if (rslt != (u32)-1)
		cout << "find !\n";

}
#endif


