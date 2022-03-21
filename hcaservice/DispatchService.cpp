#include "StdAfx.h"
#include "DispatchService.h"
#include "Utils.h"
#include "Configure.h"
#include "SqliteStoreClient.h"
#include "MessageLocationWorker.h"

DispatchService::DispatchService(void) 
	: serialClientWorker_(NULL)
	, serialWorkThread_(NULL)
	, stationWorkThread_(NULL)
	, stationClientWorker_(NULL)
	, messageEnginWorkerThread_(NULL)
	, messageLocationWorkerThread_(NULL)
	, jt808TransferThread_(NULL)
	, jt808TransferClientWorker_(NULL)
	, scheduleTaskWorker_(NULL)
	, scheduleTaskThead_(NULL)
{
}

DispatchService::~DispatchService(void)
{
	if (serialWorkThread_ != NULL) {
		serialWorkThread_->StopThread();
		delete serialWorkThread_;
		serialWorkThread_ = NULL;
	}

	if (serialClientWorker_ != NULL) {
		delete serialClientWorker_;
		serialClientWorker_ = NULL;
	}

	if(stationWorkThread_ != NULL) {
		stationWorkThread_->StopThread();
		delete stationWorkThread_;
		stationWorkThread_ = NULL;
	}

	if(stationClientWorker_ != NULL) {
		delete stationClientWorker_;
		stationClientWorker_ = NULL;
	}

	if(messageLocationWorkerThread_ != NULL) {
		messageLocationWorkerThread_->StopThread();
		delete messageLocationWorkerThread_;
		messageLocationWorkerThread_ = NULL;
	}

	if(messageEnginWorkerThread_ != NULL) {
		messageEnginWorkerThread_->StopThread();
		delete messageEnginWorkerThread_;
		messageEnginWorkerThread_ = NULL;
	}

	if(jt808TransferThread_ != NULL) {
		jt808TransferThread_->StopThread();
		delete jt808TransferThread_;
		jt808TransferThread_ = NULL;
	}

	if(jt808TransferClientWorker_ != NULL) {
		delete jt808TransferClientWorker_;
		jt808TransferClientWorker_ = NULL;
	}

	if(scheduleTaskThead_ != NULL) {
		scheduleTaskThead_->StopThread();
		delete scheduleTaskThead_;
		scheduleTaskThead_ = NULL;
	}

	if(scheduleTaskWorker_ != NULL) {
		delete scheduleTaskWorker_;
		scheduleTaskWorker_ = NULL;
	}

	m_logger.Flush();
}

DispatchService::DispatchService(const CString & name) : ServiceBase(name, 
	name,
	SERVICE_DEMAND_START,
	SERVICE_ERROR_NORMAL,
	SERVICE_ACCEPT_STOP) 
	, serialClientWorker_(NULL)
	, serialWorkThread_(NULL)
	, stationWorkThread_(NULL)
	, stationClientWorker_(NULL)
	, messageEnginWorkerThread_(NULL)
	, messageLocationWorkerThread_(NULL)
	, jt808TransferThread_(NULL)
	, jt808TransferClientWorker_(NULL)
	, scheduleTaskWorker_(NULL)
	, scheduleTaskThead_(NULL)
{
	m_logger.Init("service.log", 1024, 102400);
}

DispatchService::DispatchService(const CString & name, const CString & displayName, DWORD dwStartType) : ServiceBase(name, 
	displayName,
	dwStartType,
	SERVICE_ERROR_NORMAL,
	SERVICE_ACCEPT_STOP) 
	, serialClientWorker_(NULL)
	, serialWorkThread_(NULL)
	, stationWorkThread_(NULL)
	, stationClientWorker_(NULL)
	, messageEnginWorkerThread_(NULL)
	, messageLocationWorkerThread_(NULL)
	, jt808TransferThread_(NULL)
	, jt808TransferClientWorker_(NULL)
	, scheduleTaskWorker_(NULL)
	, scheduleTaskThead_(NULL)
{
	m_logger.Init("service.log", 1024, 102400);
}

bool DispatchService::OnStart(DWORD argc, TCHAR * argv[]) {
	if(!ReadConfig()) {
		m_logger.WriteError("the hcaservice configure file read error.\n");
		return false;
	}
	m_logger.WriteInfo("DispatchService::Application start running.\n");
	SqliteStoreClient::GetInstance().OpenDataStore();
	serialClientWorker_ = new SerialClientWorker();
	serialWorkThread_ = new IWorkThread(serialClientWorker_);
	serialWorkThread_->StartThread();

	//2、启动基站连接
	StationTransferClient::GetInstance().SetHost(Configure::GetInstance().StationHost().c_str());
	StationTransferClient::GetInstance().SetPort(Configure::GetInstance().StationPort());
	StationTransferClient::GetInstance().SetTimeOut(1, 0);
	stationClientWorker_ = new StationTransferClientWorker;
	stationWorkThread_ = new IWorkThread(stationClientWorker_);
	stationWorkThread_->StartThread();

	//3、启动808连接
	JT808TransferClient::GetInstance().SetHost(Configure::GetInstance().RemoteHost().c_str());
	JT808TransferClient::GetInstance().SetPort(Configure::GetInstance().RemotePort());
	JT808TransferClient::GetInstance().SetTimeOut(1, 0);

	jt808TransferClientWorker_ = new JT808TransferClientWorker();
	jt808TransferThread_ = new IWorkThread(jt808TransferClientWorker_);
	jt808TransferThread_->StartThread();

	//4、启动消息流转引擎
	messageEnginWorkerThread_ = new IWorkThread(&MessageEnginWorker::GetInstance());
	messageEnginWorkerThread_->StartThread();

	messageLocationWorkerThread_ = new IWorkThread(&MessageLocationWorker::GetInstance());
	messageLocationWorkerThread_->StartThread();

	//schedule task
	scheduleTaskWorker_ = new CScheduleTaskWorker;
	scheduleTaskThead_ = new IWorkThread(scheduleTaskWorker_);
	scheduleTaskThead_->StartThread();
	return true;
}

bool DispatchService::OnStop() {
	m_logger.WriteInfo("DispatchService::OnStop begin.\n");
	m_logger.Flush();
    //1、关闭基站连接
	//2、关闭808连接
	//3、关闭串口连接
	if(serialWorkThread_ != NULL) {
		serialWorkThread_->StopThread();
		delete serialWorkThread_;
		serialWorkThread_ = NULL;
	}

	if (serialClientWorker_ != NULL) {
		delete serialClientWorker_;
		serialClientWorker_ = NULL;
	}

	if(stationWorkThread_ != NULL) {
		stationWorkThread_->StopThread();
		delete stationWorkThread_;
		stationWorkThread_ = NULL;
	}

	if(stationClientWorker_ != NULL) {
		delete stationClientWorker_;
		stationClientWorker_ = NULL;
	}

	if(messageLocationWorkerThread_ != NULL) {
		messageLocationWorkerThread_->StopThread();
		delete messageLocationWorkerThread_;
		messageLocationWorkerThread_ = NULL;
	}

	if(messageEnginWorkerThread_ != NULL) {
		messageEnginWorkerThread_->StopThread();
		delete messageEnginWorkerThread_;
		messageEnginWorkerThread_ = NULL;
	}

	if(jt808TransferThread_ != NULL) {
		jt808TransferThread_->StopThread();
		delete jt808TransferThread_;
		jt808TransferThread_ = NULL;
	}

	if(jt808TransferClientWorker_ != NULL) {
		delete jt808TransferClientWorker_;
		jt808TransferClientWorker_ = NULL;
	}

	if(scheduleTaskThead_ != NULL) {
		scheduleTaskThead_->StopThread();
		delete scheduleTaskThead_;
		scheduleTaskThead_ = NULL;
	}

	if(scheduleTaskWorker_ != NULL) {
		delete scheduleTaskWorker_;
		scheduleTaskWorker_ = NULL;
	}

	SqliteStoreClient::GetInstance().CloseDataStore();
	SerialClient::GetInstance().ClosePort();
	m_logger.WriteInfo("DispatchService::OnStop end.\n");
	m_logger.Flush();
	return true;
}

bool DispatchService::OnShutdown() {
	m_logger.WriteInfo("DispatchService::OnShutdown.\n");
	m_logger.Flush();
	return OnStop();
}

bool DispatchService::OnSessionChange(DWORD evtType, WTSSESSION_NOTIFICATION * notification) {
	m_logger.WriteInfo("DispatchService::OnSessionChange.\n");
	m_logger.Flush();

	switch(evtType) {
	case WTS_SESSION_LOGON:
		break;
	case WTS_SESSION_LOGOFF:
		OnStop();
		break;
	default:
		break;
	}

	return true;
}

bool DispatchService::OnPerShutdown() {
	m_logger.WriteInfo("DispatchService::OnPerShutdown.\n");
	m_logger.Flush();
	return true;
}

bool DispatchService::OnNetBindChange(DWORD evtType) {
	m_logger.WriteInfo("DispatchService::OnNetBindChange  evtType=[%d].\n", evtType);
	m_logger.Flush();
	return true;
}

bool DispatchService::Run(LPCTSTR param /* = _T("") */) {
	if (_tcscmp(param, _T("console")) == 0) {
		TCHAR cmd[128];
		bool bStart = false;

		while (true) {
			_tprintf(_T("input the command.\n"));
			_tscanf_s(_T("%s"), cmd, 128);

			if (_tcsncmp(cmd, _T("?"), 1) == 0) {
				_tprintf(_T("\r\n========================================\r\n"));  
				_tprintf(_T("\"?\"     -show cmd help\r\n"));  
				_tprintf(_T("\"start\" -start service\r\n"));  
				_tprintf(_T("\"stop\"  -stop service\r\n"));  
				_tprintf(_T("\"exit\"  -exit service\r\n"));  
				_tprintf(_T("========================================\r\n"));  
			} else if (_tcsncmp(cmd, _T("start"), 5) == 0) {
				if (!bStart) {
					if(OnStart(0, NULL)) {
						bStart = true;
					}
				}
				_tprintf(_T("the command is start \r\n"));  
			} else if (_tcsncmp(cmd, _T("stop"), 4) == 0) {
				if (bStart) {
					OnStop();
					bStart = false;
				}
				_tprintf(_T("the command is stop \r\n"));  
			} else if (_tcsncmp(cmd, _T("exit"), 1) == 0) {
				_tprintf(_T("the command is exit \r\n"));  
				break;
			} else {
				_tprintf(_T("the command is not ok \r\n"));  
			}
		}

		if (bStart) {
			OnStop();
		}

		return true;
	}

	return ServiceBase::Run();
}

bool DispatchService::ReadConfig() {
	CString modulePath;
	if(::GetModuleFileName(NULL, modulePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH) == 0) {
		m_logger.WriteError("Couldn't get module file %d\n", ::GetLastError());
		m_logger.Flush();
		return false;
	}

	int nPos = modulePath.ReverseFind(_T('\\'));
	m_modulePath = modulePath.Left(nPos + 1);
	Configure::GetInstance().SetModulePath(Utils::CString2string(m_modulePath));

	//config path
	CString configPath(m_modulePath);
	configPath += _T("hcaservice.ini");

	TCHAR szValue[MAX_PATH] = {0}; 
	std::string output = Utils::CString2string(configPath);
	m_logger.WriteInfo("config path=[%s]\n", output.c_str());
	m_logger.Flush();

	//check the config.ini exits
	WIN32_FILE_ATTRIBUTE_DATA attrs = {0};
	DWORD dwAttribute = ::GetFileAttributesEx(configPath, ::GetFileExInfoStandard, &attrs);
	if (dwAttribute == 0) {
		m_logger.WriteError("service config.ini is not exits %d\n", ::GetLastError());
		m_logger.Flush();
		return false; 
	}

	//////////////////////////remote//////////////////////////////////
	::GetPrivateProfileString(_T("remote"), _T("host"), NULL, szValue, MAX_PATH, configPath);
	std::string remote_host = Utils::CString2string(szValue);
	Configure::GetInstance().SetRemoteHost(remote_host);
	m_logger.WriteInfo("read the remote.host=[%s]\n", remote_host.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("remote"), _T("port"), NULL, szValue, MAX_PATH, configPath);
	unsigned int remote_port = _ttoi(szValue);
	Configure::GetInstance().SetRemotePort(remote_port);
	m_logger.WriteInfo("read the remote.port=[%d]\n", remote_port);
	m_logger.Flush();
	///////////////////////////////station/////////////////////////////////////////////////////
	::GetPrivateProfileString(_T("station"), _T("host"), NULL, szValue, MAX_PATH, configPath);
	std::string station_host = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationHost(station_host);
	m_logger.WriteInfo("read the station.host=[%s]\n", station_host.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("station"), _T("port"), NULL, szValue, MAX_PATH, configPath);
	unsigned int station_port = _ttoi(szValue);
	Configure::GetInstance().SetStationPort(station_port);
	m_logger.WriteInfo("read the station.port=[%d]\n", station_port);
	m_logger.Flush();

	::GetPrivateProfileString(_T("station"), _T("mountpoint"), NULL, szValue, MAX_PATH, configPath);
	std::string station_mountpoint = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationMountPoint(station_mountpoint);
	m_logger.WriteInfo("read the station.mountpoint=[%s]\n", station_mountpoint.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("station"), _T("username"), NULL, szValue, MAX_PATH, configPath);
	std::string station_username = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationUsername(station_username);
	m_logger.WriteInfo("read the station.username=[%s]\n", station_username.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("station"), _T("password"), NULL, szValue, MAX_PATH, configPath);
	std::string station_password = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationPassword(station_password);
	m_logger.WriteInfo("read the station.password=[%s]\n", station_password.c_str());
	m_logger.Flush();
	///////////////////////////////device/////////////////////////////////
	::GetPrivateProfileString(_T("device"), _T("serialname"), NULL, szValue, MAX_PATH, configPath);
	unsigned int serial_no = _ttoi(szValue);
	Configure::GetInstance().SetSerialPortNo(serial_no);
	m_logger.WriteInfo("read the device.serialname=[COM%d]\n", serial_no);
	m_logger.Flush();

	::GetPrivateProfileString(_T("device"), _T("bandrate"), NULL, szValue, MAX_PATH, configPath);
	unsigned int serial_baud = _ttoi(szValue);
	Configure::GetInstance().SetSerialPortBaud(serial_baud);
	m_logger.WriteInfo("read the device.bandrate=[%d]\n", serial_baud);
	m_logger.Flush();

	/////////////////////////////vehicle///////////////////////////////////
	::GetPrivateProfileString(_T("vehicle"), _T("numberplate"), NULL, szValue, MAX_PATH, configPath);
	std::string numberplate = Utils::CString2string(szValue);
	Configure::GetInstance().SetNumberPlate(numberplate);
	m_logger.WriteInfo("read the vehicle.numberplate=[%s]\n", numberplate.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("phone"), NULL, szValue, MAX_PATH, configPath);
	std::string phone = Utils::CString2string(szValue);
	Configure::GetInstance().SetPhone(phone);
	m_logger.WriteInfo("read the vehicle.phone=[%s]\n", phone.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("province"), NULL, szValue, MAX_PATH, configPath);
	unsigned short privince = _ttoi(szValue);
	Configure::GetInstance().SetProvince(privince);
	m_logger.WriteInfo("read the vehicle.province=[%d]\n", privince);

	::GetPrivateProfileString(_T("vehicle"), _T("city"), NULL, szValue, MAX_PATH, configPath);
	unsigned short city = _ttoi(szValue);
	Configure::GetInstance().SetCity(city);
	m_logger.WriteInfo("read the vehicle.city=[%d]\n", city);
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("company"), NULL, szValue, MAX_PATH, configPath);
	std::string manufacturer = Utils::CString2string(szValue);
	Configure::GetInstance().SetManuFacturer(manufacturer);
	m_logger.WriteInfo("read the vehicle.company=[%s]\n", manufacturer.c_str());
	m_logger.Flush();
	::GetPrivateProfileString(_T("vehicle"), _T("model"), NULL, szValue, MAX_PATH, configPath);
	std::string model = Utils::CString2string(szValue);
	Configure::GetInstance().SetModel(model);
	m_logger.WriteInfo("read the vehicle.model=[%s]\n", model.c_str());
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("unitid"), NULL, szValue, MAX_PATH, configPath);
	std::string unitno = Utils::CString2string(szValue);
	Configure::GetInstance().SetUnitNo(unitno);
	m_logger.WriteInfo("read the vehicle.unitid=[%s]\n", unitno.c_str());
	m_logger.Flush();
	::GetPrivateProfileString(_T("vehicle"), _T("colors"), NULL, szValue, MAX_PATH, configPath);
	byte color = _ttoi(szValue);
	Configure::GetInstance().SetColor(color);
	m_logger.WriteInfo("read the vehicle.colors=[%d]\n", m_color);
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("register"), NULL, szValue, MAX_PATH, configPath);
	unsigned short need = _ttoi(szValue);
	Configure::GetInstance().SetNeedRegister(need == 1);
	JT808TransferClient::GetInstance().SetNeedRegister(need == 1);
	m_logger.WriteInfo("read the vehicle.need register=[%d]\n", need);
	m_logger.Flush();

	::GetPrivateProfileString(_T("vehicle"), _T("purchasers"), NULL, szValue, MAX_PATH, configPath);
	unsigned short purchasers = _ttoi(szValue);
	Configure::GetInstance().SetPurchasers(purchasers);
	m_logger.WriteInfo("read the vehicle.need purchasers=[%d]\n", purchasers);
	m_logger.Flush();
	return true;
}

unsigned int WINAPI DispatchService::threadProc(void* p) {
	DispatchService * pObject = (DispatchService *) p;
	pObject->DoWorker();
	return 0;
}

bool DispatchService::DoWorker() {
	HANDLE waitEvent[1];
	waitEvent[0] = m_stopEvent;

	while (WaitForMultipleObjects(1, waitEvent, FALSE, 0)) {
		/*dwRet -= WAIT_OBJECT_0;
		if (dwRet == 0) 
		{
			break;
		}

		//check the state
		if(jt808TransferThread_ != NULL) {
			jt808TransferThread_->StopThread();
		}

		if(stationWorkThread_ != NULL) {
			stationWorkThread_->StopThread();
		}
		*/
		Sleep(30000);
	}

	return true;
}
