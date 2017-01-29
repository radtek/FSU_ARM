/*
 * func.cpp
 *
 *  Created on: 2016-6-22
 *      Author: lcz
 */

#include "define.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/sysinfo.h>
#include "B_Interface.h"

float str2Float(string str) {
	const char *s = str.c_str();
	char* pEnd;
	return strtof(s, &pEnd);
}
int str2Int(string str) {
	const char *s = str.c_str();
	return atoi(s);
}
void IntToString(std::string& out, const int value) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%d", value); // snprintf is thread safe #include <stdio.h>
	out.append(buf);
}
void files() {
	// #include <dirent.h>
	//	DIR *dp;
	//	struct dirent *dirp ;
	//	while( ( dirp = readdir( dp ) ) != NULL) {
	//		if(strcmp(dirp->d_name,".")==0  || strcmp(dirp->d_name,"..")==0)
	//		  continue;
	//		int size = strlen(dirp->d_name);
	//		//如果是.wav文件，长度至少是5
	//		if(size < 9)
	//		  continue;
	//		//只存取.mp3扩展名的文件名
	//		if(strcmp( ( dirp->d_name + (size - 8) ) , ".so.mp3") != 0)
	//		  continue;
	//		*(dirp->d_name +(size - 8)) = 0;
	//		gDat.sth.push(string(dirp->d_name));
	//	 }
}
void outputInitItm(vector<string> & v) {
#if DEBUG_INIT_DAT
//	for (auto & it : v) {	// STD = C++11
	for (vector<string>::iterator it = v.begin(); it != v.end(); it++) {
//		const char * s = it.c_str();
		const char * s = (*it).c_str();
		printf("%s,", s);
	}
	cout << endl;
#endif
}
bool skipTheLine(const vector<string> &in) {
	u32 len = in.size();
	if (len == 0)
		return true;
	if ((len == 1) && (in[0].empty()))
		return true;

	return false;
}
bool checkQuit(const vector<string> &in, u32 line, const string & fn, u32 min) {
	u32 len = in.size();
	if (len < min) {
		for (auto & str : in)
			cout << str << ",";
		cout << endl;
		cout << "line ["<< line << "] size error in file:" << fn << " !" << endl;
		cout << "quit !\n";
		return true;
	}
	return false;

}
bool checkDate(int year, int mon, int day) {
	if ((year < 1900) || (year > 2100))
		return false;
	if ((mon <= 0) || (mon > 12))
		return false;
	if (day <= 0)
		return false;
	switch (mon) {
	case 2:
		if ((year % 4) == 0) {
			if (day > 29)
				return false;
		} else if (day > 28)
			return false;
		break;
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if (day > 31)
			return false;
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if (day > 30)
			return false;
		break;
	}
	return true;
}
bool checkTime(int hour, int min, int sec) {
	if ((hour < 0) || (min < 0) || (sec < 0))
		return false;
	return ((hour < 24) && (min < 60) && (sec < 60));
}
bool setSysTimeInt(int YY, int MM, int DD, int HH, int mm, int SS) {
	struct tm * stime;
	struct timeval tv;
	time_t timep, timeq;
	int rec;

	time(&timep);
	stime = localtime(&timep);

	stime->tm_sec = SS;
	stime->tm_min = mm;
	stime->tm_hour = HH;
	stime->tm_mday = DD;
	stime->tm_mon = MM - 1;
	stime->tm_year = YY - 1900;

	timeq = mktime(stime);
	tv.tv_sec = (long) timeq;
	tv.tv_usec = 0;

	rec = settimeofday(&tv, NULL);
	return (rec >= 0);
}
bool setSysTime(stDateTimeStr &tm) {
	int YY, MM, DD, HH, mm, SS;
	YY = str2Int(tm.year);
	MM = str2Int(tm.mon);
	DD = str2Int(tm.day);
	HH = str2Int(tm.hour);
	mm = str2Int(tm.min);
	SS = str2Int(tm.sec);
	if ((checkDate(YY, MM, DD)) && checkTime(HH, mm, SS)) {
		if (setSysTimeInt(YY, MM, DD, HH, mm, SS)) {
			system("hwclock  -w");
			return true;
		}
	}
	return false;
}
struct SysCPUInfo {
	char name[20];
	unsigned int user;
	unsigned int nice;
	unsigned int system;
	unsigned int idle;
};
SysCPUInfo* GetHostCPUInfo() {
	SysCPUInfo *cpuinfo = (SysCPUInfo *) malloc(sizeof(SysCPUInfo));
	if (cpuinfo == NULL) {
		log("_GetCPUInfo: malloc struct SysCPUInfo error");
	} else {
		FILE *fd;
		char buff[256];
		memset(buff, '\0', 256);

		fd = fopen("/proc/stat", "r");
		fgets(buff, sizeof(buff), fd);

		sscanf(buff, "%s %d %d %d %d", cpuinfo->name, &cpuinfo->user,
				&cpuinfo->nice, &cpuinfo->system, &cpuinfo->idle);
		fclose(fd);
	}
	return cpuinfo;
}

float _CalculateHostCPURate(SysCPUInfo *first, SysCPUInfo *second) {
	unsigned long old_CPU_Time, new_CPU_Time;
	unsigned long usr_Time_Diff, sys_Time_Diff, nic_Time_Diff;
	float cpu_use = 0.0;

	old_CPU_Time = (unsigned long) (first->user + first->nice + first->system
			+ first->idle);
	new_CPU_Time = (unsigned long) (second->user + second->nice + second->system
			+ second->idle);

	usr_Time_Diff = (unsigned long) (second->user - first->user);
	sys_Time_Diff = (unsigned long) (second->system - first->system);
	nic_Time_Diff = (unsigned long) (second->nice - first->nice);

	if ((new_CPU_Time - old_CPU_Time) != 0)
		cpu_use = (float) 100 * (usr_Time_Diff + sys_Time_Diff + nic_Time_Diff)
				/ (new_CPU_Time - old_CPU_Time);
	else
		cpu_use = 0.0;
	return cpu_use;
}

float GetHostCPURate() {
	float cpu_rate;
	SysCPUInfo *first, *second;
	first = GetHostCPUInfo();
	if (first == NULL)
		return -1;
	sleep(10);
	second = GetHostCPUInfo();
	if (second == NULL)
		return -1;

	cpu_rate = _CalculateHostCPURate(first, second);

	/* clean auxiliary memory */
	free(first);
	free(second);
	first = second = NULL;

	return cpu_rate;
}
//内存使用率
//可以直接调用Linux系统的一个库函数sysinfo（），该函数返回如下的一个结构体：
//struct sysinfo {
//	long uptime; /* Seconds since boot */
//	unsigned long loads[3]; /* 1, 5, and 15 minute load averages */
//	unsigned long totalram; /* Total usable main memory size */
//	unsigned long freeram; /* Available memory size */
//	unsigned long sharedram;/* Amount of shared memory */
//	unsigned long bufferram;/* Memory used by buffers */
//	unsigned long totalswap;/* Total swap space size */
//	unsigned long freeswap; /* swap space still available */
//	unsigned short procs; /* Number of current processes */
//	unsigned long totalhigh;/* Total high memory size */
//	unsigned long freehigh; /* Available high memory size */
//	unsigned int mem_unit; /* Memory unit size in bytes */
//	char _f[20 - 2 * sizeof(long) - sizeof(int)]; /* Padding for libc5 */
//};
//该结构体中freeram表示可用内存的大小，totalram表示总内存大小
//所以通过这两个值就可以计算内存使用率了
struct SysMemInfo {
	char name[20]; //定义一个char类型的数组名name有20个元素
	unsigned long total;
	char name2[20];
	unsigned long free;
};
SysMemInfo * GetHostMemInfo() {
	SysMemInfo *memInfo = (SysMemInfo *) malloc(sizeof(SysMemInfo));
	if (NULL == memInfo) {
		log("GetMemInfo: malloc SysMemInfo struct error");
		return NULL;
	}
	struct sysinfo tmp;
	int ret = 0;
	ret = sysinfo(&tmp);
	if (ret == 0) {
		memInfo->free = (unsigned long) tmp.freeram / (1024 * 1024);
		memInfo->total = (unsigned long) tmp.totalram / (1024 * 1024);
	} else {
		log("GetMemInfo: sysinfo() error");
	}
	return memInfo;
}
float getMemOccupyRate() {
	SysMemInfo *inf;
	inf = GetHostMemInfo();
	if (inf == NULL)
		return -1;
	float rtn = inf->free * 100 / inf->total;
	free(inf);
	return 100 - rtn;
}

string findFileNameOfPath(const string & path) {
	char name[255];
	u32 posL, posR;
	const char * s = path.c_str();
	u32 sz = path.size();
	for (u32 i = sz - 1; i >= 0; --i) {
		if(s[i] == '/') {
			posL = i;
			break;
		}
	}
	for (u32 i = sz - 1; i > posL; --i) {
		if(s[i] == '.') {
			posR = i;
			break;
		}
	}
	u32 j = 0;
	for (u32 i = posL + 1; i < posR; ++i, ++j)
		name[j] = s[i];
	name[j] = 0;
	return string(name);
}
//system("top -n 1 |grep Cpu | cut -d \",\" -f 2 >>cpu.txt");
bool GetMyCPURate(string & out) {
	system("top -n 1 |grep CPU: | cut -d \",\" -f 1 | cut -d \":\" -f 2 >.cpu");
	char buf[16];
	int fd = open(".cpu", O_RDONLY);
	if (-1 != fd) {
		int res = read(fd, buf, 16);
		for (int i = 0; i < res; ++i) {
			if (*(buf + i) == '%') {
				*(buf + i) = 0;
				out = string(buf);
				close(fd);
				return true;
			}
		}
	}
	return false;
}
bool GetMyMemRate(string & out) {
	system("top -n 1 |grep Mem | cut -d \",\" -f 1 | cut -d \":\" -f 2 >.totalmem");
	char buf[16];
	int used;
	int fd = open(".totalmem", O_RDONLY);
	bool usedOk = false;
	if (-1 != fd) {
		int res = read(fd, buf, 16);
		for (int i = 0; i < res; ++i) {
			if (*(buf + i) == 'K') {
				*(buf + i) = 0;
				used = atoi(buf);
				usedOk = true;
				close(fd);
			}
		}
	}
	if (usedOk) {
		int free;
		system("top -n 1 |grep Mem | cut -d \",\" -f 2 >.usedmem");
		char buf[16];
		int fd = open(".usedmem", O_RDONLY);
		if (-1 != fd) {
			int res = read(fd, buf, 16);
			for (int i = 0; i < res; ++i) {
				if (*(buf + i) == 'K') {
					*(buf + i) = 0;
					free = atoi(buf);
					float occ = (float)used / (used + free);
					out = floatToString(occ * 100, 2, 1);
					close(fd);
					return true;
				}
			}
		}
	}


	return false;
}
string floatToString(double fZ, const int slen, const int alen) {
	char tmpstr[16];

	memset(tmpstr, '\0', 16);
	sprintf(tmpstr, "%*.*lf", slen, alen, fZ); //-- 这里控制精度
	return (string(tmpstr));
}
void getAbsTime(timespec & t, u32 timeout_ms) {
	struct timeval now;
	gettimeofday(&now, NULL);
	int nsec = now.tv_usec * 1000 + (timeout_ms % 1000) * 1000000;
	t.tv_nsec = nsec % 1000000000;
	t.tv_sec = now.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
}
double getDouble(const char * s) {
	typedef union {
		u32 Word;				            //字操作定义
		struct {
			u32 M :23;		    //
			u32 E :8;		        //
			u32 S :1;		        //

		} Bits;
		struct {
			u32 Byte1 :8;		         //	low
			u32 Byte2 :8;		         //
			u32 Byte3 :8;		         //
			u32 Byte4 :8;		         //	high
		} Bits_8;
	} uDataFlow;
	uDataFlow Dataflow;
	double tmp;
	Dataflow.Bits_8.Byte1 = *(s + 3);
	Dataflow.Bits_8.Byte2 = *(s + 2);
	Dataflow.Bits_8.Byte3 = *(s + 1);
	Dataflow.Bits_8.Byte4 = *s;
	tmp = Dataflow.Bits.M;
	tmp = (0x00000001 << (Dataflow.Bits.E - 127)) * (1 + tmp / 0x00800000) * 10;
	return tmp;
}

bool isStrDatOk(string str) {
	const char *s = str.c_str();
	u32 sz = str.size();
	u32 nPoint = 0;
	for (u32 i = 0; i < sz; ++i) {
		if (*(s + i) >= '0' && (*(s + i) <= '9')) {
		} else if (*(s + i) == '.') {
			if (++nPoint > 1) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}
bool checkAndChangeDateFormat(string & s) {
	unsigned int sz = s.size();
	if ((sz < 10) || (sz > 19))
		return false;
	char ch[20] = { '2', '0', '0', '0', '/', '0', '1', '/', '0', '1', ' ', '0',
			'0', ':', '0', '0', ':', '0', '0', 0 };
	unsigned int i;
	const char * t = s.c_str();
	for (i = 0; i < sz; ++i)
		ch[i] = t[i];
	ch[4] = '-';
	ch[7] = '-';
	ch[10] = ' ';
	s = string(ch);
	return true;
}
char Char2Num(char c) {
	char ch = 0;
	if (c > 0x60)
		c -= 0x20;
	if (c < 0x3a)
		ch = c - 0x30;
	if (c > 0x40)
		ch = c - 0x37;
	ch &= 0x0f;
	return ch;
}
u8 Ch2_2_Num(char c1, char c2) {
	return ((Char2Num(c1) << 4) + Char2Num(c2));
}
#if 0
char Num2Char(char num)
{
	char ch = num & 0x0f;
	return ch + ((ch <= 9) ? 0x30 : 0x37);
}
void Char2DblChar(u8 in, char* pOut)
{
	u8 f,s;
	f = in;
	s = in & 0x0f;
	f = (f >> 4)&0x0f;
	*pOut = Num2Char(f);
	*(pOut+1)= Num2Char(s);
}
u16 Ch4_2_Num(char c1, char c2, char c3, char c4)
{
	u16 Hi,Lo;

	Hi = Ch2_2_Num(c1, c2);
	Lo = Ch2_2_Num(c3, c4);

	return ((Hi << 8) + Lo);
}
#endif
bool readCsv(vector<vector<string> > & data, string & filename) {
#if DEBUG_CSV
	cout << "readCsv() - read file:" << filename << endl;
#endif
	ifstream in(filename);
	string element, delimiters = ",\n\r";
	int row = 0;
	char ch;
	bool empty = true;
	data.push_back(vector<string>());
	while (in.read((char*) &ch, 1)) {
		empty = false;
		if (delimiters.find_first_of(ch) == delimiters.npos)
			element += ch;
		else {
			if (ch != '\r') {
				data[row].push_back(element);
				element = "";

				if (ch == '\n') {
					data.push_back(vector<string>());
					row++;
				}
			}
		}
	}
	if (element.size() > 0)
		data[row].push_back(element);
	u32 sz = data.size();
	if (empty){
		cout << "empty file or file not found. \n";
		return false;
	} else {
		if ((data[sz - 1].size() == 0) ||
		 ((data[sz - 1].size() == 1) && (data[sz - 1][0].size() == 0)))
			data.pop_back();
	}

	in.close();
#if DEBUG_CSV
	cout << "csv dat : ---- start ----" << endl;
	for (auto & s : data) {
		for (auto & ss: s) {
			cout << ss << "\t|";
		}
		cout << endl;
	}
	cout << " ---- end ----" << endl;
#endif
	return true;
}
bool writeCsv(string & filename, const vector<vector<string> > &vvStr) {
// 1、判断是否是CSV文件
//	if (!IsCsvFile(lpszFilename))
//	return XC_ERR_INVALID_FILE_NAME;
// 2、打开CSV文件
	ofstream _streamToFile(filename);
//    判断打开文件是否成功
	if (NULL == _streamToFile)
		return false;
// 本地变量
	static char chQuote = '"';
	static char chComma = ',';
// Loop through each list of string in vector
	for (vector<vector<string> >::const_iterator vIt = vvStr.begin();
			vIt != vvStr.end(); vIt++) {
// Loop through each string in list
		for (vector<string>::const_iterator lIt = vIt->begin();
				lIt != vIt->end(); lIt++) {
// Separate this value from previous
			if (vIt->begin() != lIt)
				_streamToFile.put(chComma);
// 考虑string中可能有,或"的情况，这就要特殊包装。
			bool bComma = (lIt->find(chComma) != lIt->npos);
			bool bQuote = (lIt->find(chQuote) != lIt->npos);
// 真的含有,或"的情况
			if (bComma || bQuote) {
				_streamToFile.put(chQuote);

				if (bQuote) {
					for (string::const_iterator chIt = lIt->begin();
							chIt != lIt->end(); chIt++) {
// Pairs of quotes interpreted as single quote
						if (chQuote == *chIt)
							_streamToFile.put(chQuote);

						_streamToFile.put(*chIt);
					}
				} else
					_streamToFile << *lIt;

				_streamToFile.put(chQuote);
			} else
				_streamToFile << *lIt;
		}
// 换行
		_streamToFile << endl;
	}
//
	return true;		         //XC_ERR_NONE;
}
int IsFolderExist(const char* path) {
	DIR *dp;
	if ((dp = opendir(path)) == NULL) {
		return 0;
	}

	closedir(dp);
	return -1;
}
//template <typename T>
//void InsertSort(u32 arr[],int count) {
//    int i,j,temp;
//    for(i = 1; i < count; ++i) {
//        temp = arr[i];
//        for(j = i - 1; (j>-1) && (arr[j]>temp); j--) {
//            arr[i] = arr[j];
//            arr[j] = temp;
//        }
//    }
//}
void InsertSort(u32 a[],int n)//n为数组a的元素个数
{
    //进行N-1轮插入过程
    for(int i=1; i<n; i++)
    {
        //首先找到元素a[i]需要插入的位置
        int j=0;
        while( (a[j]<a[i]) && (j<i))
        {
            j++;
        }
        //将元素插入到正确的位置
        if(i != j)  //如果i==j，说明a[i]刚好在正确的位置
        {
            int temp = a[i];
            for(int k = i; k > j; k--)
            {
                a[k] = a[k-1];
            }
            a[j] = temp;
        }
    }
}

//检查文件(所有类型)是否存在
//-1:存在 0:不存在
int IsFileExist(const char* path) {
	return !access(path, F_OK);
}
int ipaddr(string & ip)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
     *               can free list later */

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
         *                   form of the latter for the common families) */
#if 0
        printf("%s  address family: %d%s\n",
                ifa->ifa_name, family,
                (family == AF_PACKET) ? " (AF_PACKET)" :
                (family == AF_INET) ?   " (AF_INET)" :
                (family == AF_INET6) ?  " (AF_INET6)" : "");
#endif
        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                    sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
//            printf("\taddress: <%s>\n", host);
            ip = host;
        }
    }

    freeifaddrs(ifaddr);
    return 1;
}
void printAsc(unsigned char * pBuf, u32 len) {
	cout << pBuf << endl;
}
void printHex(unsigned char * pBuf, u32 len) {
	unsigned int k;
	for (k = 0; k < len; ++k)
		printf("%02X ", *(pBuf+k));
	printf("\n");
}
void printHexNo_n(unsigned char * pBuf, u32 len) {
	unsigned int k;
	for (k = 0; k < len; ++k)
		printf(" %02X", *(pBuf+k));
}
