#ifndef  _HCASERVICE_DISPATCH_SERVICE_H_
#define  _HCASERVICE_DISPATCH_SERVICE_H_

#include "Event.h"
#include "Logger.h"
#include "ServiceBase.h"
#include "ServiceInstall.h"
#include "SerialClientWorker.h"
#include "StationTransferClient.h"
#include "MessageEnginWorker.h"
#include "JT808TransferClient.h"
#include "ScheduleTaskWorker.h"

class DispatchService : public ServiceBase
{
public:
	DispatchService(void);
	~DispatchService(void);
	DispatchService(const CString & name);
	DispatchService(const CString & name, const CString & displayName, DWORD dwStartType);

public:
	bool Run(LPCTSTR param = _T("service"));

private:
	bool OnStop();
	bool OnShutdown();
	bool OnPerShutdown();
	bool OnNetBindChange(DWORD evtType);
	bool OnStart(DWORD argc, TCHAR * argv[]);
	bool OnSessionChange(DWORD evtType, WTSSESSION_NOTIFICATION * notification);

private:
	bool ReadConfig();

public:
	static unsigned int WINAPI threadProc(void* p);
	bool   DoWorker();

private:
	Logger  m_logger;

	//程序路径
	CString    m_modulePath;
	//配置文件项
	std::string m_remote_host;
	unsigned int m_remote_port;

	//基站配置
	std::string m_station_host;
	unsigned int m_station_port;
	std::string m_station_mountpoint;
	std::string m_station_username;
	std::string m_station_password;

	//串口配置
	unsigned int m_serial_no;
	unsigned int m_serial_baud;

	//车辆配置
	std::string m_numberplate;
	std::string m_phone;
	WORD m_privince;
	WORD m_city;
	std::string m_compnay;
	std::string m_model;
	std::string m_unitid;
	byte m_color;

private:
	IWorkThread  * serialWorkThread_;
	SerialClientWorker * serialClientWorker_;
	IWorkThread  * stationWorkThread_;
	StationTransferClientWorker * stationClientWorker_;

	IWorkThread  * messageEnginWorkerThread_;
	IWorkThread  * messageLocationWorkerThread_;

	JT808TransferClientWorker * jt808TransferClientWorker_;
	IWorkThread  * jt808TransferThread_;

	CScheduleTaskWorker * scheduleTaskWorker_; 
	IWorkThread * scheduleTaskThead_;


	Event m_stopEvent;
	DISABLE_COPY_AND_MOVE(DispatchService)
};

#endif /*_HCASERVICE_DISPATCH_SERVICE_H_*/