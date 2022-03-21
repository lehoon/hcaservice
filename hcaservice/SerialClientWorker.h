#ifndef _HCASERVICE_SERIALCLIENT_WORKER_H_
#define _HCASERVICE_SERIALCLIENT_WORKER_H_

#include "Singleton.h"
#include "IWorkThread.h"
#include "SerialPort.h"
#include "Logger.h"
#include "base_type.h"
#include <time.h>

class SerialClient : public CSerialPort, public Singleton<SerialClient>
{
public:
	SerialClient(void);
	~SerialClient(void);

public:
	SERIALPORT_STATE state() { return m_state; }
	void State(SERIALPORT_STATE state) {m_state = state;}
};

class SerialClientWorker : public IThreadUser
{
	typedef void (SerialClientWorker::*EventFunction)();
public:
	SerialClientWorker(void);
	~SerialClientWorker(void);

private:
	bool Process();
	void SetDeviceMode();
	bool ReConnectionDevice();

public:
	void ConnectEventFunction();
	void ReadDataEventFunction();

public:
	bool InitTask();
	bool ExitTask();
	bool Work();
private:
	Logger        m_logger;
	Logger        m_ggaLogger;

	unsigned char m_buffer[4096];
	unsigned int  m_index;
	unsigned int  m_length;

	GPGGA         m_gga;
	GPRMC         m_rmc;

	time_t        m_lasttime;

	SERIALPORT_STATE     m_state;
	EventFunction	     m_EventMap[0x4];
};
#endif /*_HCASERVICE_SERIALCLIENT_WORKER_H_*/