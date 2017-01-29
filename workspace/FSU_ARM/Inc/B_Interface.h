/*
 * B_Interface.h
 *
 *  Created on: 2016-5-7
 *      Author: lcz
 */

#ifndef B_INTERFACE_H_
#define B_INTERFACE_H_

#include "define.h"
#include <string>
#include <vector>

using namespace std;

namespace BInt {

static const string DEV_TYPE_FSU = string("38");

//只能小于
static const u32 SZ_NAME_LENGTH = 40; //名字命名长度		 40字节
static const u32 SZ_USER_LENGTH = 20; //用户名长度		 20字节
static const u32 SZ_PASSWORD_LEN = 20; //口令长度			 20字节
static const u32 SZ_EVENT_LENGTH = 160; //事件信息长度		160字节
static const u32 SZ_ALARM_LENGTH = 165; //告警事件信息长度	165字节
static const u32 SZ_LOGIN_LENGTH = 100; //登录事件信息长度	100字节
static const u32 SZ_DES_LENGTH = 40; //描述信息长度		 40字节
static const u32 SZ_UNIT_LENGTH = 8; //数据单位的长度		  8字节
static const u32 SZ_STATE_LENGTH = 160; //态值描述长度		160字节
static const u32 SZ_VER_LENGTH = 20; //版本描述的长度		 20字节
static const u32 SZ_IP_LENGTH = 15; //IP串长度			 15字节

//固定长度
static const u32 SZ_AREACODE_LENGTH = 7; //区域编码长度		  7字节
static const u32 SZ_STATIONCODE_LENGTH = 12; //机房编码长度		 12字节
static const u32 SZ_NODECODE_LENGTH = 11; //监控信号编码		 11字节
static const u32 SZ_FSUID_LEN = 14; //FSU ID字符串长度	 14字节
static const u32 SZ_FSUCODE_LEN = 14; //FSU编码字符串长度	 14字节
static const u32 SZ_DEVICEID_LEN = 14; //设备ID长度		 14字节
static const u32 SZ_DEVICECODE_LEN = 14; //设备编码			 14字节
static const u32 SZ_ID_LENGTH = 10; //监控点ID长度		 10字节
static const u32 SZ_SERIALNO_LEN = 10; //告警序号长度		 10字节
static const u32 SZ_TIME_LEN = 19; //时间串长度		 19字节

static const u32 T_SEMA_DI = 0;
static const u32 T_SEMA_AI = 1;
static const u32 T_SEMA_DO = 2;
static const u32 T_SEMA_AO = 3;
static const int NAME_LENGTH = 40; //名字命名长度		 40字节
static const int USER_LENGTH = 20; //用户名长度		 20字节
static const int PASSWORD_LEN = 20; //口令长度			 20字节
static const int EVENT_LENGTH = 160; //事件信息长度		160字节
static const int ALARM_LENGTH = 165; //告警事件信息长度	165字节
static const int LOGIN_LENGTH = 100; //登录事件信息长度	100字节
static const int DES_LENGTH = 40; //描述信息长度		 40字节
static const int UNIT_LENGTH = 8; //数据单位的长度		  8字节
static const int STATE_LENGTH = 160; //态值描述长度		160字节
static const int VER_LENGTH = 20; //版本描述的长度		 20字节
static const int AREACODE_LENGTH = 7; //区域编码长度		  7字节
static const int STATIONCODE_LENGTH = 12; //机房编码长度		 12字节
static const int NODECODE_LENGTH = 11; //监控信号编码		 11字节
static const int FSUID_LEN = 14; //FSU ID字符串长度	 14字节
static const int FSUCODE_LEN = 14; //FSU编码字符串长度	 14字节
static const int IP_LENGTH = 15; //IP串长度			 15字节
static const int DEVICEID_LEN = 14; //设备ID长度		 14字节
static const int DEVICECODE_LEN = 14; //设备编码			 14字节
static const int ID_LENGTH = 10; //监控点ID长度		 10字节
static const int SERIALNO_LEN = 10; //告警序号长度		 10字节
static const int TIME_LEN = 19; //时间串长度		 19字节
static const int PORT_LENGTH = 5;
static const int SITECODE_LENGTH = 6;
static const int SITETYPE_LENGTH = 1;
static const int FSU_COMMTYPE_LEN = 1;
static const int MODE_LENGTH = 1;
static const int BAUD_LEN=7;

static const u32 PK_IDX_101_LOGIN = 0;
static const u32 PK_IDX_102_LOGIN_ACK = 1;
static const u32 PK_IDX_103_LOGOUT = 2;
static const u32 PK_IDX_104_LOGOUT_ACK = 3;
static const u32 PK_IDX_401_GET_DATA = 4;
static const u32 PK_IDX_402_GET_DATA_ACK = 5;
static const u32 PK_IDX_403_GET_HISDATA = 6;
static const u32 PK_IDX_404_GET_HISDATA_ACK = 7;
static const u32 PK_IDX_501_SEND_ALARM = 8;
static const u32 PK_IDX_502_SEND_ALARM_ACK = 9;
static const u32 PK_IDX_1001_SET_POINT = 10;
static const u32 PK_IDX_1002_SET_POINT_ACK = 11;
static const u32 PK_IDX_1301_TIME_CHECK = 12;
static const u32 PK_IDX_1302_TIME_CHECK_ACK = 13;
static const u32 PK_IDX_1501_GET_LOGININFO = 14;
static const u32 PK_IDX_1502_GET_LOGININFO_ACK = 15;
static const u32 PK_IDX_1503_SET_LOGININFO = 16;
static const u32 PK_IDX_1504_SET_LOGININFO_ACK = 17;
static const u32 PK_IDX_1601_GET_FTP = 18;
static const u32 PK_IDX_1602_GET_FTP_ACK = 19;
static const u32 PK_IDX_1603_SET_FTP = 20;
static const u32 PK_IDX_1604_SET_FTP_ACK = 21;
static const u32 PK_IDX_1701_GET_FSUINFO = 22;
static const u32 PK_IDX_1702_GET_FSUINFO_ACK = 23;
static const u32 PK_IDX_1801_SET_FSUREBOOT = 24;
static const u32 PK_IDX_1802_SET_FSUREBOOT_ACK = 25;
static const u32 PK_IDX_1901_GET_THRESHOLD = 26;
static const u32 PK_IDX_1902_GET_THRESHOLD_ACK = 27;
static const u32 PK_IDX_2001_SET_THRESHOLD = 28;
static const u32 PK_IDX_2002_SET_THRESHOLD_ACK = 29;

enum class EnumRightMode:char { //监控系统FSU向SC提供的权限定义
	INVALID,//无权限
	LEVEL1,//具备数据读的权限,当用户可以读某个数据，而无法写任何数据时返回这一权限值。
	LEVEL2//具备数据读、写的权限，当用户对某个数据具有读写权限时返回这一权限值。
};
enum class EnumResult:char { //报文返回结果
	FAILURE = 0, //失败
	SUCCESS = 1 //成功
};
enum class EnumType { //监控系统数据的种类
	STATION, //局、站
	DEVICE, //设备
	DI, //数字输入量（包含多态数字输入量）
	AI, //模拟输入量
	DO, //数字输出量
	AO, //模拟输出量
	AREA = 9 //区域
};
//static char ** vecStrType = {
// "0", //局、站
//"1", //设备
//"2", //数字输入量（包含多态数字输入量）
//"3", //模拟输入量
//"4", //数字输出量
//"5", //模拟输出量
//"",
//"",
//"",
//"9" //区域
//};

bool getEnumType(const char * c, EnumType &t);
enum class EnumAlarmLevel { //告警的等级
	NOALARM, //无告警
	CRITICAL, //一级告警
	MAJOR, //二级告警
	MINOR, //三级告警
	HINT //四级告警
};
enum class EnumEnable { //使能的属性
	DISABLE, //禁止/不能
	ENABLE //开放/能
};
enum class EnumAcceSCMode { //实时数据访问的方式
	ASK_ANSWER, //一问一答方式
	CHANGE_TRIGGER, //改变时自动发送数据方式
	TIME_TRIGGER, //定时发送数据方式
	STOP //停止发送数据方式
};
enum class EnumState { //数据值的状态
	NOALARM, //正常数据
	CRITICAL, //一级告警
	MAJOR, //二级告警
	MINOR, //三级告警
	HINT, //四级告警
	OPEVENT, //操作事件
	INVALID //无效数据
};
EnumState getEnumState(const char * c);
enum class EnumFlag { //告警标志
	BEGIN, //开始
	END //结束
};
enum class EnumAlarmMode { //告警等级设定的模式
	NOALARM, //不做告警上报
	CRITICAL, //一级告警上报
	MAJOR, //二级告警上报
	MINOR, //三级告警上报
	HINT //四级告警上报
};
enum class EnumStationType { //局站类型
	SP, //特殊机房（自定义）
	AC, //A级机房
	BC, //B级机房
	CC, //C级机房
	DC, //D级机房
	RC1, //保留
	RC2, //保留
	RC3, //保留
	RC4, //保留
	RC5 //保留
};
enum class EnumModifyType { //对象属性修改类型
	ADDNONODES, //新增（无子节点）
	ADDINNODES, //新增（含子节点）
	DELETE, //删除
	MODIFYNONODES, //修改（仅修改本节点）
	MODIFYINNODES //修改（涉及到子节点）
};
enum class EnumDeviceType { //设备类型
	DEV_TYPE1 = 1, //高压配电
	DEV_TYPE2, //低压配电
	DEV_TYPE3, //交流配电屏
	DEV_TYPE4, //直流配电屏
	DEV_TYPE5, //柴油发电机组
	DEV_TYPE6, //开关电源
	DEV_TYPE7, //蓄电池组
	DEV_TYPE8, //UPS设备
	DEV_TYPE9, //UPS配电屏
	DEV_TYPE10, //UPS电池
	DEV_TYPE11, //240V直流系统
	DEV_TYPE12, //专用空调(风冷)
	DEV_TYPE13, //中央空调(水冷)
	DEV_TYPE14, //专用空调（通冷冻水型）
	DEV_TYPE15, //普通空调
	DEV_TYPE16, //智能电表
	DEV_TYPE17, //门禁系统
	DEV_TYPE18, //机房/基站环境
	DEV_TYPE19, //监控设备
	DEV_TYPE20, //太阳能/风能设备
	DEV_TYPE21, //燃气轮机发电机组
	DEV_TYPE22, //风力发电设备
	DEV_TYPE23, //智能通风系统
	DEV_TYPE24, //新风设备
	DEV_TYPE25, //热交换设备
	DEV_TYPE26, //热管设备
	DEV_TYPE27, //蓄电池温控柜
	DEV_TYPE28, //防雷设备/防雷箱
	DEV_TYPE29, //燃料电池
	DEV_TYPE30, //模块化UPS
	DEV_TYPE31, //240V电池
	DEV_TYPE32, //铁塔
	DEV_TYPE33 //FSU
//94~99			//预留
};
//static const u32 DEV_TYPE_02_JiaoLiuPeiDianXiang  	= 2;
static const u32 DEV_TYPE_06_KaiGuanDianYuan 		= 6;
static const u32 DEV_TYPE_07_BattMeas 				= 7;
//static const u32 DEV_TYPE_16_ZhiNengDianBiao 		= 16;
static const u32 DEV_TYPE_17_DoorGuard 				= 17;
static const u32 DEV_TYPE_18_JiFangHuanJing  		= 18;
static const u32 DEV_TYPE_19_TongXinZhongDuan  		= 19;
static const u32 SIG_TYPE_01_IllegalEntry			= 1;
static const u32 SIG_TYPE_01_AI_MagneticDoor		= 1;
static const u32 SIG_TYPE_03_INFRA					= 3;
static const u32 CNT_INFRA_DOOR						= 3;
/*
 static const int PKC_101_LOGIN 				= 101;	//SC<—FSU//注册
 static const int PKC_102_LOGIN_ACK 			= 102;	//SC—>FSU//注册响应
 static const int PKC_103_LOGOUT				= 103;	//SC<—FSU//登出
 static const int PKC_104_LOGOUT_ACK			= 104;	//SC—>FSU//登出响应
 static const int PKC_401_GET_DATA			= 401;	//SC—>FSU//用户请求监控点数据
 static const int PKC_402_GET_DATA_ACK		= 402;	//SC<—FSU//用户请求监控点数据响应
 static const int PKC_403_GET_HISDATA		= 403;	//SC—>FSU//用户请求监控点历史数据
 static const int PKC_404_GET_HISDATA_ACK	= 404;	//SC—>FSU//用户请求监控点历史数据响应
 static const int PKC_501_SEND_ALARM			= 501;	//SC<—FSU//实时告警发送
 static const int PKC_502_SEND_ALARM_ACK		= 502;	//SC—>FSU//实时告警发送确认
 static const int PKC_1001_SET_POINT			= 1001;	//SC—>FSU//写数据请求
 static const int PKC_1002_SET_POINT_ACK		= 1002;	//SC<—FSU//写数据响应
 static const int PKC_1301_TIME_CHECK		= 1301;	//SC—>FSU//发送时钟消息
 static const int PKC_1302_TIME_CHECK_ACK	= 1302;	//SC<—FSU//时钟同步响应
 static const int PKC_1501_GET_LOGININFO		= 1501;	//SC—>FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
 static const int PKC_1502_GET_LOGININFO_ACK	= 1502;	//SC<—FSU//获取注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
 static const int PKC_1503_SET_LOGININFO		= 1503;	//SC—>FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）
 static const int PKC_1504_SET_LOGININFO_ACK	= 1504;	//SC<—FSU//设置注册信息（IPSec用户、密码、IPSec服务器IP、SC IP数据）响应
 static const int PKC_1601_GET_FTP			= 1601;	//SC—>FSU//获取FSU的FTP用户、密码数据
 static const int PKC_1602_GET_FTP_ACK		= 1602;	//SC<—FSU//获取FSU的FTP用户、密码数据响应
 static const int PKC_1603_SET_FTP			= 1603;	//SC—>FSU//设置FSU的FTP用户、密码数据
 static const int PKC_1604_SET_FTP_ACK		= 1604;	//SC<—FSU//设置FSU的FTP用户、密码数据响应
 static const int PKC_1701_GET_FSUINFO		= 1701;	//SC—>FSU//获取FSU的状态参数
 static const int PKC_1702_GET_FSUINFO_ACK	= 1702;	//SC<—FSU//获取FSU的状态参数响应
 static const int PKC_1801_SET_FSUREBOOT		= 1801;	//SC—>FSU//重启FSU
 static const int PKC_1802_SET_FSUREBOOT_ACK	= 1802;	//SC<—FSU//重启FSU响应
 static const int PKC_1901_GET_THRESHOLD		= 1901;	//SC—>FSU//用户请求监控点门限数据
 static const int PKC_1902_GET_THRESHOLD_ACK	= 1902;	//SC<—FSU//用户请求监控点门限数据响应
 static const int PKC_2001_SET_THRESHOLD		= 2001;	//SC—>FSU//用户请求写监控点门限数据请求
 static const int PKC_2002_SET_THRESHOLD_ACK	= 2002;	//SC<—FSU//用户请求写监控点门限数据响应

 static const char PKS_101_LOGIN 				[] = "LOGIN";
 static const char PKS_102_LOGIN_ACK 			[] = "LOGIN_ACK";
 static const char PKS_103_LOGOUT				[] = "LOGOUT";
 static const char PKS_104_LOGOUT_ACK			[] = "LOGOUT_ACK";
 static const char PKS_401_GET_DATA				[] = "GET_DATA";
 static const char PKS_402_GET_DATA_ACK			[] = "GET_DATA_ACK";
 static const char PKS_403_GET_HISDATA			[] = "GET_HISDATA";
 static const char PKS_404_GET_HISDATA_ACK		[] = "GET_HISDATA_ACK";
 static const char PKS_501_SEND_ALARM			[] = "SEND_ALARM";
 static const char PKS_502_SEND_ALARM_ACK		[] = "SEND_ALARM_ACK";
 static const char PKS_1001_SET_POINT			[] = "SET_POINT";
 static const char PKS_1002_SET_POINT_ACK		[] = "SET_POINT_ACK";
 static const char PKS_1301_TIME_CHECK			[] = "TIME_CHECK";
 static const char PKS_1302_TIME_CHECK_ACK		[] = "TIME_CHECK_ACK";
 static const char PKS_1501_GET_LOGININFO		[] = "GET_LOGININFO";
 static const char PKS_1502_GET_LOGININFO_ACK	[] = "GET_LOGININFO_ACK";
 static const char PKS_1503_SET_LOGININFO		[] = "SET_LOGININFO";
 static const char PKS_1504_SET_LOGININFO_ACK	[] = "SET_LOGININFO_ACK";
 static const char PKS_1601_GET_FTP				[] = "GET_FTP";
 static const char PKS_1602_GET_FTP_ACK			[] = "GET_FTP_ACK";
 static const char PKS_1603_SET_FTP				[] = "SET_FTP";
 static const char PKS_1604_SET_FTP_ACK			[] = "SET_FTP_ACK";
 static const char PKS_1701_GET_FSUINFO			[] = "GET_FSUINFO";
 static const char PKS_1702_GET_FSUINFO_ACK		[] = "GET_FSUINFO_ACK";
 static const char PKS_1801_SET_FSUREBOOT		[] = "SET_FSUREBOOT";
 static const char PKS_1802_SET_FSUREBOOT_ACK	[] = "SET_FSUREBOOT_ACK";
 static const char PKS_1901_GET_THRESHOLD		[] = "GET_THRESHOLD";
 static const char PKS_1902_GET_THRESHOLD_ACK	[] = "GET_THRESHOLD_ACK";
 static const char PKS_2001_SET_THRESHOLD		[] = "SET_THRESHOLD";
 static const char PKS_2002_SET_THRESHOLD_ACK	[] = "SET_THRESHOLD_ACK";
 */
//EnumDeviceCode	设备编码	见设备编码表	见设备编码表
/*	设备/系统类型	设备编码（EnumDeviceCode）
 A类局站	B类局站	C类局站	                                                                                                            D类局站
 1	高压配电				101		201		301		401
 2	低压配电 				102 	202 	302 	402
 3	交流配电屏 			103 	203 	303 	403
 4	直流配电屏 			104 	204 	304 	404
 5	柴油发电机组 			105 	205 	305 	405
 6	开关电源 				106 	206 	306 	406
 7	蓄电池组 				107 	207 	307 	407
 8	UPS设备 				108 	208 	308 	408
 9	UPS配电屏 			109 	209 	309 	409
 10	UPS电池 				110 	210 	310 	410
 11	240V直流系统 			111 	211 	311 	411
 12	专用空调(风冷) 		112 	212 	312 	412
 13	中央空调(水冷) 		113 	213 	313 	413
 14	专用空调（通冷冻水型） 	114 	214 	314 	414
 15	普通空调 				115 	215 	315 	415
 16	智能电表 				116 	216 	316 	416
 17	门禁系统 				117 	217 	317 	417
 18	机房/基站环境 			118 	218 	318 	418
 19	监控设备 				119 	219 	319 	419
 20	太阳能/风能设备 		120 	220 	320 	420
 21	燃气轮机发电机组 		121 	221 	321 	421
 22	风力发电设备 			122 	222 	322 	422
 23	智能通风系统 			123 	223 	323 	423
 24	新风设备 				124 	224 	324 	424
 25	热交换设备 			125 	225 	325 	425
 26	热管设备 				126 	226 	326 	426
 27	蓄电池温控柜 			127 	227 	327 	427
 28	防雷设备/防雷箱 		128 	228 	328 	428
 29	燃料电池				129		229 	329 	429
 30	模块化UPS				130		230		330		430
 31	240V电池				131		231		331		431
 32	铁塔					132		232		332		432
 33	FSU					133		233		333		433
 34-99	预留
 */

struct stTime { //时间的结构
	short years; //年
	char month; //月
	char day; //日
	char Hour; //时
	char minute; //分
	char second; //秒
};
struct stSemaphore { //信号量的值的结构
	stSemaphore(string ty, string i)
	: type(ty) , id(i) ,measuredVal(string()), setupVal(string()), status(string()) {}
	stSemaphore() : measuredVal(string()), setupVal(string()), status(string()) {}
	string type; //数据类型
	string id; //监控点ID
	string measuredVal; //实测值
	string setupVal; //设置值
	string status; //状态
	string recordTime; //记录时间，YYYY-MM-DD<SPACE键>hh:mm:ss（采用24小时的时间制式），取历史数据时的记录时间
};
struct stSemaphore_l { //信号量的值的结构 less ,无记录时间
	stSemaphore_l(const string &ty, const string &_id, const string &v, const string &st)
		: type(ty), id(_id), measuredVal(v), setupVal(string()), status(st) {}
	stSemaphore_l(const string & _id) : id(_id) {}
	stSemaphore_l() {}
	string type; //数据类型
	string id; // char[ID_LENGTH];	//监控点ID
	string measuredVal; //实测值
	string setupVal; //设置值
	string status; //状态
};
struct stAlarmDB {
	u32 sn;
	string id;
	string deviceId;
	string deviceCode;
	string date;
	string time;
	string level;
	string flag;
	string desc;
};
struct stDevAlmDB {
	string devId;
	vector <stAlarmDB> v;
};
struct stThreshold { //信号量的门限值的结构
	string type; //数据类型
	string id; //监控点ID
	string Threshold; //门限值
	string AbsoluteVal; //绝对阀值
	string RelativeVal; //百分比阀值
	string status; //状态
};
struct stAlarm { //当前告警值的结构
	string serialNo;//[SERIALNO_LEN]; //告警序号
	string id;//[ID_LENGTH]; //监控点ID
	string fsuId;//[FSUID_LEN]; //FSU ID号，资源系统的ID
	string fsuCode;//[FSUCODE_LEN]; //FSU 编码
	string deviceId;//[DEVICEID_LEN]; //设备ID
	string deviceCode;//[DEVICECODE_LEN]; //设备编码
	string alarmTime;//[DES_LENGTH]; //告警时间，YYYY-MM-DD<SPACE键>hh:mm:ss（采用24小时的时间制式）
	string alarmLevel; //告警级别EnumState
	string alarmFlag; //告警标志EnumFlag
	string alarmDesc;//[DES_LENGTH]; //告警的事件描述
};
struct stGPS { //GPS数据结构
	long fsuId; //FSU ID
	float lag; //经度
	float lat; //纬度
};
struct stFSUStatus { //FSU状态参数
	float CPUUsage; //CPU使用率
	float MEMUsage; //内存使用率
};
struct stIdVal {
	stIdVal(string _id, string _val) : id(_id), val(_val) {}
	string id;
	string val;
};

struct devIdCode {
	string Id;
	string Code;
};
struct stInitGRP {
	stInitGRP(const string &sId, const string &gId);
	string semaId;
	string grpId;
//---------------------------
	bool bGrp;
	u32 grpNum;
	bool bFixNum;
};
struct stInit4ADIO {
	stInit4ADIO(/*const string &dId,*/ const string &sId, const string &val, const string &nm) :
			/*devId(dId), */semaId(sId), setVal(val), name(nm) {
	}
//	string devId;
	string semaId;
	string setVal;
	string name;
};
struct stInitDI2 {
	stInitDI2(string dlyA,string dlyB,string dlyC,string dlyD,
			string thrA,string thrB,string thrC,string thrD,
			string lvlA,string lvlB,string lvlC,string lvlD)
	: delayA(dlyA), delayB(dlyB), delayC(dlyC), delayD(dlyD),
	  thresholdA(thrA), thresholdB(thrB),thresholdC(thrC),thresholdD(thrD),
	  levelA(lvlA), levelB(lvlB), levelC(lvlC), levelD(lvlD) {}
	string delayA;
	string delayB;
	string delayC;
	string delayD;
	string thresholdA;
	string thresholdB;
	string thresholdC;
	string thresholdD;
	string levelA;
	string levelB;
	string levelC;
	string levelD;

};
struct stInitDI {
	stInitDI(const string &Id,
			const string &aId,
			const string &lgc,
			const string &bl,
			const string &nm,
			const string &onStr,
			const string &offStr,
			const string &dly,
			const string &lvl,
			const string &thrd,
			stInitGRP * ig, string u)
	: id(Id),
	  refAIId(aId),
	  logic(lgc),
	  backlash(bl),
	  name(nm),
	  almOnDesc(onStr),
	  almOffDesc(offStr),
	  delay_sel(dly),
	  level_sel(lvl),
	  threshold_sel(thrd),
	  refAiUnit(u)
	{
		if (ig) {
			gId = ig->grpId;
			bGrp = true;
			grpNum = ig->grpNum;
			bFixNum = ig->bFixNum;
		} else {
			bGrp = false;
			grpNum = 0;
		}
	}
	string id;
	string refAIId;
	string logic;
	string backlash;
	string name;
	string almOnDesc;
	string almOffDesc;
	string delay_sel;
	string level_sel;
	string threshold_sel;
	string gId;
	string refAiUnit;
	bool bGrp;
	u32 grpNum;
	bool bFixNum;
	bool bAvail = false;
};
struct stInitAI {
	stInitAI(const string &Id,
			const string &abs,
			const string &rel,
			const string &nm,
			const string &u,
			stInitGRP * ig)
	: id(Id),
	  threshold_abs(abs),
	  threshold_rel(rel),
	  name(nm),
	  unit(u) {
		if (ig) {
			gId = ig->grpId;
			bGrp = true;
			grpNum = ig->grpNum;
			bFixNum = ig->bFixNum;
		} else {
			bGrp = false;
			grpNum = 0;
		}
	}
	string id;
	string threshold_abs;
	string threshold_rel;
	string name;
	string unit;
	string gId;
	bool bGrp;
	u32 grpNum;
	bool bFixNum;
	bool bAvail = false;
};
struct stInitAO {
	stInitAO(const string &sId,
			const string & nm,
			const string & u,
			stInitGRP * ig)
	: semaId(sId),
	  name(nm),
	  unit(u)
		{
		if (ig) {
			gId = ig->grpId;
			bGrp = true;
			grpNum = ig->grpNum;
			bFixNum = ig->bFixNum;
		} else {
			bGrp = false;
			grpNum = 0;
		}
	}
	string semaId;
	string name;
	string unit;
	string gId;
	bool bFixNum;
	bool bGrp;
	u32 grpNum;
	bool bAvail = false;
};
struct stInitDO {
	stInitDO(const string &sId,
			const string &nm,
			stInitGRP * ig) :
		semaId(sId), name(nm) {
		if (ig) {
			gId = ig->grpId;
			bGrp = true;
			grpNum = ig->grpNum;
			bFixNum = ig->bFixNum;
		} else {
			bGrp = false;
			grpNum = 0;
		}
	}
	string semaId;
	string name;
	string gId;
	bool bFixNum;
	bool bGrp;
	u32 grpNum;
	bool bAvail = false;
};
//struct stInitComm {
//	string id;
//	string code;
//	string portType;
//	string portIdx;
//	u32 baudrate;
//	u8 databit;
//	char parity;
//	string stopbit;
//	u32 maxdelay;
//	string type;	// file
//	string format;	// file
//	string ver;
//	string addr;
//	u32 bufSize;
//	bool bAvail = false;
//};
struct thrdItm {
	thrdItm(const string &d, const string &s, const string &t, const string & nm) :
		devId(d), semaId(s), thrd(t), name(nm) {}
	string devId;
	string semaId;
	string thrd;
	string name;
};
struct setThresholdReponseOfDevice {
	string devId;
//	vector<pair<string,string>> successIds;
	vector<thrdItm> vecSuccessItms;
	vector<string> failedIds;
};
struct setPointReponseOfDevice {
	string devId;
	vector<string> successIds;
	vector<string> failedIds;
};

bool strLenHdl(u32 type, string &in, string & out);
bool isIdAvailable(string id, u32 len);
bool isIdAvailable_sema(string id);
bool isSameSemaType(const string & l, const string r);
bool isFloat(string val);// 可带小数点的数值
//void fmtSemaId(string & id);
string & fmtSemaId1(string & id);
bool chkFmtSemaId(string & id);
void getDevTypeStr(const string & devId, string & out); // devId必须是7位的数字
void getDevTypeStr_fullId(const string & devId, string & out); // devId必须是14位的数字
u32 getDevType(const string & devId); // devId必须是7位的数字
u32 getSemaphoreDevType(const string & sId);// sId 必须是检查过的8位数字
u32 getSemaphoreType(const string & sId); 	// sId 必须是检查过的8位数字
u32 getSemaphoreIdx(const string & sId); 	// sId 必须是检查过的8位数字
u32 getSemaphoreIdx2(const string & sId); 	// sId 必须是检查过的8位数字
u32 getSemaphoreDevType_fullId(const string & sId);	// sId 必须是检查过的10位数字
u32 getSemaphoreType_fullId(const string & sId); 	// sId 必须是检查过的10位数字
u32 getSemaphoreIdx_fullId(const string & sId); 	// sId 必须是检查过的10位数字
u32 getSemaphoreIdx2_fullId(const string & sId); 	// sId 必须是检查过的10位数字
u32 getDeviceTypeVal(const string & devId) ;
string getDeviceTypeStr(const string & devId) ;
inline u32 getSemaphoreDevType(const string & sId) {	// sId 必须是检查过的8位数字
	const char * s = sId.c_str();
	return (s[0] - '0') * 10 + (s[1] - '0');
}
inline u32 getSemaphoreType(const string & sId) { 		// sId 必须是检查过的8位数字
	const char * s = sId.c_str();
	return s[2] - '0';
}
inline u32 getSemaphoreIdx(const string & sId) { 		// sId 必须是检查过的8位数字
	const char * s = sId.c_str();
	return (s[3] - '0') * 10 + (s[4] - '0');
}
inline u32 getSemaphoreIdx2(const string & sId) { 		// sId 必须是检查过的8位数字
	const char * s = sId.c_str();
	return (s[5] - '0') * 100 + (s[6] - '0') * 10 + (s[7]) - '0';
}
inline string getSemaphoreShortId(const string & sId) {	// sId 必须是检查过的10位数字
	const char * s = sId.c_str();
	if (s[2] == '0')
		return string(s + 3);
	else
		return string(s + 2);
}
inline u32 getSemaphoreDevType_fullId(const string & sId) {	// sId 必须是检查过的10位数字
	const char * s = sId.c_str();
	return (s[2] - '0') * 10 + (s[3] - '0');
}
inline u32 getSemaphoreType_fullId(const string & sId) { 		// sId 必须是检查过的10位数字
	const char * s = sId.c_str();
	return s[4] - '0';
}
inline u32 getSemaphoreIdx_fullId(const string & sId) { 		// sId 必须是检查过的10位数字
	const char * s = sId.c_str();
	return (s[5] - '0') * 10 + (s[6] - '0');
}
inline u32 getSemaphoreIdx2_fullId(const string & sId) { 		// sId 必须是检查过的10位数字
	const char * s = sId.c_str();
	return (s[7] - '0') * 100 + (s[8] - '0') * 10 + (s[9]) - '0';
}
inline u32 getDeviceTypeVal(const string & devId) { // devId 必须是检查过的14位数字
	const char * s = devId.c_str();
	return (s[7] - '0') * 10 + (s[8] - '0');
}
inline string getDeviceTypeStr(const string & devId) { // devId 必须是检查过的14位数字
	const char * s = devId.c_str();
	char _id[3] = { s[7], s[8], 0 };
	return string(_id);
}
string semaIdCalcAdd(const string & semaId, u32 i);// 8位数字
void getSemaIdx2Str(const string & semaId, string & out); // just last 2 char !!!
ostream& operator <<(ostream& out, EnumType tp);
ostream& operator <<(ostream& out, EnumState st);
ostream& operator <<(ostream& out, EnumFlag f);

}
#endif /* B_INTERFACE_H_ */
