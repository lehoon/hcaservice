#include "stdafx.h"
#include <time.h>

#include "SerialClientWorker.h"
#include "NemaFormatProtocol.h"
#include "MessageLocationWorker.h"
#include "Configure.h"
#include "Utils.h"


static inline unsigned long mktime (unsigned int year, unsigned int mon,
	unsigned int day, unsigned int hour,
	unsigned int min, unsigned int sec);
static inline void mktime_pos(PPosition pos);

SerialClient::SerialClient(void)
{
}


SerialClient::~SerialClient(void){

}

SerialClientWorker::SerialClientWorker() 
	: m_index(0)
	, m_length(0)
{
}

SerialClientWorker::~SerialClientWorker(void) {
}

bool SerialClientWorker::InitTask() {
	std::string logPath = Configure::GetInstance().ModulePath() + "serialop.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	logPath = Configure::GetInstance().ModulePath() + "ggalogger.log";
	m_ggaLogger.Init(logPath.c_str(), 1024, 102400);
	m_EventMap[SERIAL_STATE_NO_CONNECT] = &SerialClientWorker::ConnectEventFunction;
	m_EventMap[SERIAL_STATE_CONNECT_SUCCESS] = &SerialClientWorker::ReadDataEventFunction;
	time(&m_lasttime);
	return true;
}

bool SerialClientWorker::ExitTask() {
	m_logger.Flush();
	m_ggaLogger.Flush();
	return true;
}

bool SerialClientWorker::Work() {
	(this->*m_EventMap[SerialClient::GetInstance().state()])();
	return true;
}

bool SerialClientWorker::Process() {
	if(m_length < 8) {
		return false;
	}

	int nIndex = 0;
	int nType = -1;
	int nLength = 0;
	int result = NemaFormatProtocol::GetInstance().ParserDataFrame(m_buffer, m_length, nIndex, nLength, nType);

	if(result == -1) {
		return false;
	}

	if(result == -2) {
		m_index = 0;
		m_length = 0;
		return false;
	}

	if(result == -3 || result == -4 || result == -5) {
		if(nIndex > 0) {
			m_length -= nIndex;
			m_index = m_length;
			memmove(m_buffer, m_buffer + nIndex, m_length);
		}
		return false;
	}

	unsigned char buffer[1024] = {0};
	memcpy(buffer, m_buffer + nIndex, nLength > 1023 ? 1023 : nLength);

	if(nType == 0) {
		MessageLocationWorker::GetInstance().SendPostion(buffer, nLength);
		NemaFormatProtocol::GetInstance().ParserGPGGA(buffer, nLength, &m_gga);
	} else {
		NemaFormatProtocol::GetInstance().ParserGPRMC(buffer, nLength, &m_rmc);
	}

	//移动内存指针，维护数据模型
	int nEnd = nIndex + nLength;
	m_length -= nEnd;
	m_index = m_length;

	if(m_length > 0) {
		memmove(m_buffer, m_buffer + nEnd, m_length);
	}
	return true;
}

bool SerialClientWorker::ReConnectionDevice() {
	SerialClient::GetInstance().State(SERIAL_STATE_NO_CONNECT);
	m_logger.WriteError("serialport close, now reconnect...\n");
	return true;
}

void SerialClientWorker::ConnectEventFunction() {
	//1、关闭串口连接
	SerialClient::GetInstance().ClosePort();
	//2、启动串口连接
	bool bResult = SerialClient::GetInstance().InitPort(Configure::GetInstance().SerialPortNo(), Configure::GetInstance().SerialPortBaud(), 'N', 8, 1, EV_RXCHAR);

	if(bResult) {
		SerialClient::GetInstance().State(SERIAL_STATE_CONNECT_SUCCESS);
		SetDeviceMode();
	} else {
		m_logger.WriteInfo("open the serial port error. error=[%d]\n", GetLastError());
	}

	Sleep(100);
}

void SerialClientWorker::ReadDataEventFunction() {
	int count = SerialClient::GetInstance().GetBytesInCOM();
	if (count == 0) {
		time_t now;
		time(&now);

		if ((now - m_lasttime) > 3) {
			ReConnectionDevice();
		}

		Sleep(1000);
		return;
	}

	char ch = 0;
	time(&m_lasttime);
	while (count-- > 0) {
		if (m_index >= 4096){
			m_index = 0;
			m_length = 0;
		}
		SerialClient::GetInstance().ReadChar(ch);
		m_buffer[m_index++] = ch;
		m_length++;
	}

	//记录从串口读取的数据
	char tempBuffer[4096] = {0};
	memcpy(tempBuffer, m_buffer, m_length);
	m_ggaLogger.WriteDebug("read from serial m_length=[%d] buffer=[%s] \n", m_length, tempBuffer);

	while (Process()){
		//找到一组匹配的数据
		if(m_gga.hour == m_rmc.hour && m_gga.minute == m_rmc.minute && m_gga.second == m_rmc.second) {
			if(m_gga.state == 0 || m_rmc.state == 0) {
				continue;
			}

			Position * pos = new Position;
			pos->type = REAL_TYPE;
			pos->year = m_rmc.year;
			pos->month = m_rmc.month;
			pos->day = m_rmc.day;
			pos->hour = m_rmc.hour;
			pos->minute = m_rmc.minute;
			pos->second = m_rmc.second;
			pos->latitude = m_rmc.latitude;
			pos->latdir = m_rmc.latdir;
			pos->longitude = m_rmc.longitude;
			pos->londir = m_rmc.londir;
			pos->course = m_rmc.course;
			pos->speed = m_rmc.speed;
			pos->state = m_rmc.state;
			pos->diff = m_gga.state;
			pos->starnum = m_gga.starnum;
			pos->alt = m_gga.alt;
			if(pos->hour > 15) {
				mktime_pos(pos);
			} else {
				pos->hour += 8;
			}

			m_logger.WriteInfo("GPGGA,20%02d%02d%02d%02d%02d%02d,%d,%d,%d,%d,%d,%d\n", pos->year, pos->month, pos->day, pos->hour, pos->minute, pos->second,
				pos->latitude, pos->longitude, pos->state, pos->diff, pos->starnum, pos->speed);
			MessageLocationWorker::GetInstance().PushPosition(pos);
		}
	}

	Sleep(1000);
}

void SerialClientWorker::SetDeviceMode(){
	char command[16] = {0};
	strncpy_s(command, "log gpgga\r\n", 15);
	SerialClient::GetInstance().WriteData((unsigned char *) command, strlen(command));
	strncpy_s(command, "unlog gpatt\r\n", 15);
	SerialClient::GetInstance().WriteData((unsigned char *) command, strlen(command));
}

static inline unsigned long mktime (unsigned int year, unsigned int mon,
	unsigned int day, unsigned int hour,
	unsigned int min, unsigned int sec)
{
	if (0 >= (int) (mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
		mon += 12;      /* Puts Feb last since it has leap day */
		year -= 1;
	}

	return (((
		(unsigned long) (year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
		year * 365 - 719499
		) * 24 + hour /* now have hours */
		) * 60 + min /* now have minutes */
		) * 60 + sec; /* finally seconds */
}

static inline void mktime_pos(PPosition pos) {
	//calc the second count
	unsigned long times = mktime(pos->year + 2000, pos->month, pos->day, pos->hour, pos->minute, pos->second);
	time_t postime = times + 28800;
	struct tm gpstime;
	gmtime_s(&gpstime, (const time_t *) &postime);

	pos->year = gpstime.tm_year - 100;
	pos->month = gpstime.tm_mon + 1;
	pos->day = gpstime.tm_mday;
	pos->hour = gpstime.tm_hour;
	pos->minute = gpstime.tm_min;
	pos->second = gpstime.tm_sec;
}