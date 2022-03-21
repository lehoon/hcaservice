#ifndef _HCASERVICE_STATIONTRANSFER_CLIENT_H_
#define _HCASERVICE_STATIONTRANSFER_CLIENT_H_

#include "Logger.h"
#include "SerialClientWorker.h"
#include "IWorkThread.h"
#include "TcpTransferBase.h"
#include "Singleton.h"

enum NtripState{
	NTRIP_STATE_NO_CONNECT = 0,
	NTRIP_STATE_CONNECT_SUCCESS,
	NTRIP_STATE_WAIT_AUTHOR_RESPONSE,
	NTRIP_STATE_LOGIN_SUCCESS
};

class StationTransferClient : public TcpTransferBase, public Singleton<StationTransferClient>
{
public:
	StationTransferClient(void);
	~StationTransferClient(void);

public:
	int  state() { return m_state; }
	time_t statetm() {return m_statetm; }
	void State(NtripState state);

public:
	bool SendRealTimeLocation(unsigned char * buffer, int & length);

private:
	time_t       m_statetm;
	NtripState   m_state;
	Logger       m_logger;
};

class StationTransferClientWorker : public IThreadUser
{
	typedef bool (StationTransferClientWorker::*EventFunction) ();
public:
	StationTransferClientWorker(void);
	~StationTransferClientWorker(void);

public:
	void SetHost(const char * addr);
	void SetPort(const int port);
	void SetTimeOut(unsigned int second, unsigned int usec);
	void SetSerialPort(CSerialPort * serialPort);
public:
	bool InitTask();
	bool ExitTask();
	bool Work();

public:
	bool ConnectServerFunction();
	bool SendAuthorFunction();
	bool WaitAuthorFunction();
	bool LoginSuccessFunction();

public:
	bool Process();

private:
	Logger m_logger;
	//client state and action map
	EventFunction	      m_EventMap[0x8];
};

#endif /*_HCASERVICE_STATIONTRANSFER_CLIENT_H_*/