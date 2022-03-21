#ifndef _HCASERVICE_BASE_TYPES_H
#define _HCASERVICE_BASE_TYPES_H

#include <memory>
#include <string>

#define MAIN_VERSION            0x0001

#define PROTOCOL_PACKAGE_BEGIN  0x7e
#define PROTOCOL_PACKAGE_END    0x7e

//GGA
typedef struct _Stu_GPGGA_ {
	unsigned int hour;    //ʱ
	unsigned int minute;  //��
	unsigned int second;  //��
	unsigned int state;   //��״̬
	unsigned int starnum; //������
	unsigned short alt;    //�߳�
} GPGGA, *PGPGGA;

//RMC
typedef struct _Stu_GPRMC_ {
	unsigned int hour;    //ʱ
	unsigned int minute;  //��
	unsigned int second;  //��

	unsigned int state;   //��״̬

	int speed;             //�ٶ�
	int course;            //����

	unsigned int year;
	unsigned int month;
	unsigned int day;

	int latitude;
	int longitude;

	unsigned char latdir;  //��γ����γ
	unsigned char londir;  //����������
} GPRMC, *PGPRMC;

enum {
	REAL_TYPE,
	OLD_TYPE
};

typedef struct _POSITION_STU {
	unsigned int type;    //0 realtype, 1oldtype
	unsigned int hour;    //ʱ
	unsigned int minute;  //��
	unsigned int second;  //��

	unsigned int starnum; //������
	unsigned int state;   //��״̬
	unsigned int diff;    //���״̬

	int speed;             //�ٶ�
	int course;            //����

	unsigned int year;
	unsigned int month;
	unsigned int day;


	unsigned short alt;    //�߳�

	int latitude;
	int longitude;

	unsigned char latdir;  //��γ����γ
	unsigned char londir;  //����������
} Position, *PPosition;

enum {
	PACKAGE_GPGGA,
	PACKAGE_GPRMC
};

struct Message {
	unsigned char   timeout;//timeout
	unsigned short  len;    //row���ݳ���
	unsigned short  ref;    //ref count
	unsigned short  seqno;  //sequeue no
	unsigned short  use_len;
	unsigned int    type;
	time_t			lasttime;
	unsigned char * data;   //����
};

#endif /*_HCASERVICE_BASE_TYPES_H*/