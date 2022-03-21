#ifndef _HCASERVICE_BASE_TYPES_H
#define _HCASERVICE_BASE_TYPES_H

#include <memory>
#include <string>

#define MAIN_VERSION            0x0001

#define PROTOCOL_PACKAGE_BEGIN  0x7e
#define PROTOCOL_PACKAGE_END    0x7e

//GGA
typedef struct _Stu_GPGGA_ {
	unsigned int hour;    //时
	unsigned int minute;  //分
	unsigned int second;  //秒
	unsigned int state;   //解状态
	unsigned int starnum; //卫星数
	unsigned short alt;    //高程
} GPGGA, *PGPGGA;

//RMC
typedef struct _Stu_GPRMC_ {
	unsigned int hour;    //时
	unsigned int minute;  //分
	unsigned int second;  //秒

	unsigned int state;   //解状态

	int speed;             //速度
	int course;            //航向

	unsigned int year;
	unsigned int month;
	unsigned int day;

	int latitude;
	int longitude;

	unsigned char latdir;  //北纬、南纬
	unsigned char londir;  //东经、西经
} GPRMC, *PGPRMC;

enum {
	REAL_TYPE,
	OLD_TYPE
};

typedef struct _POSITION_STU {
	unsigned int type;    //0 realtype, 1oldtype
	unsigned int hour;    //时
	unsigned int minute;  //分
	unsigned int second;  //秒

	unsigned int starnum; //卫星数
	unsigned int state;   //解状态
	unsigned int diff;    //差分状态

	int speed;             //速度
	int course;            //航向

	unsigned int year;
	unsigned int month;
	unsigned int day;


	unsigned short alt;    //高程

	int latitude;
	int longitude;

	unsigned char latdir;  //北纬、南纬
	unsigned char londir;  //东经、西经
} Position, *PPosition;

enum {
	PACKAGE_GPGGA,
	PACKAGE_GPRMC
};

struct Message {
	unsigned char   timeout;//timeout
	unsigned short  len;    //row数据长度
	unsigned short  ref;    //ref count
	unsigned short  seqno;  //sequeue no
	unsigned short  use_len;
	unsigned int    type;
	time_t			lasttime;
	unsigned char * data;   //数据
};

#endif /*_HCASERVICE_BASE_TYPES_H*/