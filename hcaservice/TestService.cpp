
#include "stdafx.h"

#include <iostream>
#include <time.h>

#include "LehoonService.h"
#include "Utils.h"
#include "SqliteStoreClient.h"
#include "StationTransferClient.h"
#include "Configure.h"
#include "JT808TransferClient.h"
#include "JT808FormatProtocol.h"
#include "SerialClientWorker.h"
#include "Logger.h"

void testStationClient();
void testSqliteDbClient();
void testJt808Client();
void testSerialClient();
void read_config();
void testLocalMessageEnginer();
void testLogger();

int  test_service_console(int argc, _TCHAR * argv[]);

int _tmain(int argc, _TCHAR * argv[]) {
	TcpTransferBase::InitSocketEnv();
	//read_config();
	//testJt808Client();
	//testSerialClient();
	//testSqliteDbClient();
	test_service_console(argc, argv);
	//testStationClient();

	//testLogger();
	return 0;
}

int  test_service_console(int argc, _TCHAR * argv[]) {
	CString name = "LehoonService";
	LehoonService service(name);
	if (argc > 1) {
		if (_tcscmp(argv[1], _T("install")) == 0) {
			OutputDebugString(_T("_tmain install. \n"));
			if(!ServiceInstall::Install(service)) {
			}
		} else if (_tcscmp(argv[1], _T("uninstall")) == 0) {
			OutputDebugString(_T("_tmain uninstall. \n"));
			if (!ServiceInstall::UnInstall(service)) {
			}
		} else if (_tcscmp(argv[1], _T("console")) == 0)  {
			OutputDebugString(_T("_tmain console. \n"));
			service.Run(_T("console"));
		} else if (_tcscmp(argv[1], _T("start")) == 0)  {
			OutputDebugString(_T("_tmain console. \n"));
			service.Run(_T("start"));
		} else if (_tcscmp(argv[1], _T("stop")) == 0)  {
			OutputDebugString(_T("_tmain console. \n"));
			service.Run(_T("stop"));
		}
	} else {
		OutputDebugString(_T("_tmain service. \n"));
		service.Run();
	}

	return 0;
}

void testStationClient() {
	std::string host = "121.42.37.33";
	std::string mountpoint = "RTCM32";
	std::string username = "txbd2";
	std::string password = "123";

	Configure::GetInstance().SetStationHost(host);
	Configure::GetInstance().SetStationMountPoint(mountpoint);
	Configure::GetInstance().SetStationUsername(username);
	Configure::GetInstance().SetStationPassword(password);
	Configure::GetInstance().SetStationPort(2101);

	StationTransferClient::GetInstance().SetHost(host.c_str());
	StationTransferClient::GetInstance().SetPort(2101);
	StationTransferClientWorker * clientWorker = new StationTransferClientWorker();
	clientWorker->InitTask();

	IWorkThread * workThread = new IWorkThread(clientWorker);
	workThread->StartThread();
	Sleep(200000);
	workThread->StopThread();
	delete clientWorker;
	delete workThread;
}

void testSqliteDbClient() {
	SqliteStoreClient::GetInstance().OpenDataStore();
	SqliteStoreClient::GetInstance().LoadStorePosition();
	Sleep(20000);
	SqliteStoreClient::GetInstance().CloseDataStore();
}

void testJt808Client() {
	JT808TransferClient::GetInstance().SetHost(Configure::GetInstance().RemoteHost().c_str());
	JT808TransferClient::GetInstance().SetPort(Configure::GetInstance().RemotePort());
	//JT808TransferClient::GetInstance().ConnectJT808Server();
	JT808TransferClientWorker * jt808ClientWorker = new JT808TransferClientWorker();

	IWorkThread * clienThread = new IWorkThread(jt808ClientWorker);
	bool bStartFlag = clienThread->StartThread();
	cout<<"808 client thread start flag="<<bStartFlag<<endl;

	while(JT808TransferClient::GetInstance().state() != STATE_LOGIN_SUCCESS) {
		Sleep(1000);
		break;
	}

	cout<<"808 client login in server..."<<endl;
	time_t begin, end;
	time(&begin);
	int count = 0;

	while(true) {
		time(&end);
		if(end - begin > 5) {
			JT808TransferClient::GetInstance().SendHeartBeatRequest();
			begin = end;
			count++;
		}

		if(count > 5000) break;
		Sleep(100);
	}

	Sleep(10000);
	clienThread->StopThread();
	delete jt808ClientWorker;
	delete clienThread;
}

void testSerialClient() {
	//1、启动串口连接
	bool bResult = SerialClient::GetInstance().InitPort(Configure::GetInstance().SerialPortNo(), Configure::GetInstance().SerialPortBaud(), 'N', 8, 1, EV_RXCHAR);

	if(!bResult) {
		return;
	}



	SqliteStoreClient::GetInstance().OpenDataStore();
	SerialClientWorker * serialClientWorker_ = new SerialClientWorker();
	IWorkThread * serialWorkThread_ = new IWorkThread(serialClientWorker_);
	serialWorkThread_->StartThread();
	Sleep(100000);
	serialWorkThread_->StopThread();
	delete serialWorkThread_;
	delete serialClientWorker_;
}

void testLocalMessageEnginer() {

}

//读取配置文件
void read_config() {
	CString modulePath;
	if(::GetModuleFileName(NULL, modulePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH) == 0) {
		_tprintf(_T("Couldn't get module file name:%d\n"), ::GetLastError());
		return;
	}

	int nPos = modulePath.ReverseFind(_T('\\'));
	CString strModulePath = modulePath.Left(nPos + 1);
	Configure::GetInstance().SetModulePath(Utils::CString2string(strModulePath));

	//config path
	CString configPath(strModulePath);
	configPath += _T("config.ini");

	TCHAR szValue[MAX_PATH] = {0}; 

	std::string output = Utils::CString2string(configPath);

	//////////////////////////remote//////////////////////////////////
	::GetPrivateProfileString(_T("remote"), _T("host"), NULL, szValue, MAX_PATH, configPath);
	std::string remote_host = Utils::CString2string(szValue);
	Configure::GetInstance().SetRemoteHost(remote_host);


	::GetPrivateProfileString(_T("remote"), _T("port"), NULL, szValue, MAX_PATH, configPath);
	unsigned int remote_port = _ttoi(szValue);
	Configure::GetInstance().SetRemotePort(remote_port);

	///////////////////////////////station/////////////////////////////////////////////////////
	::GetPrivateProfileString(_T("station"), _T("host"), NULL, szValue, MAX_PATH, configPath);
	std::string station_host = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationHost(station_host);

	::GetPrivateProfileString(_T("station"), _T("port"), NULL, szValue, MAX_PATH, configPath);
	unsigned int station_port = _ttoi(szValue);
	Configure::GetInstance().SetStationPort(station_port);

	::GetPrivateProfileString(_T("station"), _T("mountpoint"), NULL, szValue, MAX_PATH, configPath);
	std::string station_mountpoint = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationMountPoint(station_mountpoint);

	::GetPrivateProfileString(_T("station"), _T("username"), NULL, szValue, MAX_PATH, configPath);
	std::string station_username = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationUsername(station_username);

	::GetPrivateProfileString(_T("station"), _T("password"), NULL, szValue, MAX_PATH, configPath);
	std::string station_password = Utils::CString2string(szValue);
	Configure::GetInstance().SetStationPassword(station_password);

	///////////////////////////////device/////////////////////////////////
	::GetPrivateProfileString(_T("device"), _T("serialname"), NULL, szValue, MAX_PATH, configPath);
	unsigned int serial_no = _ttoi(szValue);
	Configure::GetInstance().SetSerialPortNo(serial_no);

	::GetPrivateProfileString(_T("device"), _T("bandrate"), NULL, szValue, MAX_PATH, configPath);
	unsigned int serial_baud = _ttoi(szValue);
	Configure::GetInstance().SetSerialPortBaud(serial_baud);

	/////////////////////////////vehicle///////////////////////////////////
	::GetPrivateProfileString(_T("vehicle"), _T("numberplate"), NULL, szValue, MAX_PATH, configPath);
	std::string numberplate = Utils::CString2string(szValue);
	Configure::GetInstance().SetNumberPlate(numberplate);

	::GetPrivateProfileString(_T("vehicle"), _T("phone"), NULL, szValue, MAX_PATH, configPath);
	std::string phone = Utils::CString2string(szValue);
	Configure::GetInstance().SetPhone(phone);
	//JT808TransferClient::GetInstance().SetRegister(true);
	//JT808TransferClient::GetInstance().SetAuthCode(phone);


	::GetPrivateProfileString(_T("vehicle"), _T("province"), NULL, szValue, MAX_PATH, configPath);
	unsigned short privince = _ttoi(szValue);
	Configure::GetInstance().SetProvince(privince);

	::GetPrivateProfileString(_T("vehicle"), _T("city"), NULL, szValue, MAX_PATH, configPath);
	unsigned short city = _ttoi(szValue);
	Configure::GetInstance().SetCity(city);

	::GetPrivateProfileString(_T("vehicle"), _T("company"), NULL, szValue, MAX_PATH, configPath);
	std::string manufacturer = Utils::CString2string(szValue);
	Configure::GetInstance().SetManuFacturer(manufacturer);
	
	::GetPrivateProfileString(_T("vehicle"), _T("model"), NULL, szValue, MAX_PATH, configPath);
	std::string model = Utils::CString2string(szValue);
	Configure::GetInstance().SetModel(model);

	::GetPrivateProfileString(_T("vehicle"), _T("unitid"), NULL, szValue, MAX_PATH, configPath);
	std::string unitno = Utils::CString2string(szValue);
	Configure::GetInstance().SetUnitNo(unitno);

	::GetPrivateProfileString(_T("vehicle"), _T("colors"), NULL, szValue, MAX_PATH, configPath);
	byte color = _ttoi(szValue);
	Configure::GetInstance().SetColor(color);

	::GetPrivateProfileString(_T("vehicle"), _T("register"), NULL, szValue, MAX_PATH, configPath);
	unsigned short need = _ttoi(szValue);
	Configure::GetInstance().SetNeedRegister(need == 1);
	JT808TransferClient::GetInstance().SetNeedRegister(need == 1);
}

void testLogger() {
	Logger logger;
	logger.Init("testlog.log", 512, 51200);

	int count = 0;
	while (count++ < 1000000) {
		logger.WriteDebug("heloo lehoon..........\n");
		Sleep(10);
	}

	logger.Flush();
}

