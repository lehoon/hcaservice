#include "stdafx.h"

#include "StationTransferClient.h"
#include "NtripFormatProtocol.h"
#include "Configure.h"

StationTransferClient::StationTransferClient(void) 
	: m_state(NTRIP_STATE_NO_CONNECT)
{
	std::string logPath = Configure::GetInstance().ModulePath() + "stationclient.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
}

StationTransferClient::~StationTransferClient(void)
{
	m_logger.Flush();
	Close();
	m_state = NTRIP_STATE_NO_CONNECT;
}

bool StationTransferClient::SendRealTimeLocation(unsigned char * gga, int & length) {
	if(m_state != NTRIP_STATE_LOGIN_SUCCESS || gga == NULL || length == 0) return false;
	if(SendTimeOut(30)) {
		m_logger.WriteInfo("send the realtime location to ntrip server length=[%d] [%s]\n", length, gga);
		char buffer[512] = {0};
		int outLength = NtripFormatProtocol::GetInstance().EncodeNmeaRequest(Configure::GetInstance().StationMountPoint().c_str(), (const char *) gga, buffer, 511);
		SendData((unsigned char *) buffer, outLength);
	}
	return true;
}

void StationTransferClient::State(NtripState state) { 
	m_state = state; 
	time(&m_statetm);
}

StationTransferClientWorker::StationTransferClientWorker(){
}

StationTransferClientWorker::~StationTransferClientWorker() {
}

bool StationTransferClientWorker::InitTask() {
	m_EventMap[NTRIP_STATE_NO_CONNECT] = &StationTransferClientWorker::ConnectServerFunction;
	m_EventMap[NTRIP_STATE_CONNECT_SUCCESS] = &StationTransferClientWorker::SendAuthorFunction;
	m_EventMap[NTRIP_STATE_WAIT_AUTHOR_RESPONSE] = &StationTransferClientWorker::WaitAuthorFunction;
	m_EventMap[NTRIP_STATE_LOGIN_SUCCESS] = &StationTransferClientWorker::Process;

	std::string logPath = Configure::GetInstance().ModulePath() + "stationclientt-worker.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	return true;
}

bool StationTransferClientWorker::ExitTask() {
	StationTransferClient::GetInstance().Close();
    m_logger.Flush();
	return true;
}

bool StationTransferClientWorker::Work() {
	(this->*m_EventMap[StationTransferClient::GetInstance().state()])();
	return true;
}

bool StationTransferClientWorker::Process() {
	int code = StationTransferClient::GetInstance().IsReadable();

	if(code == -1) {
		Sleep(1000);
		StationTransferClient::GetInstance().Close();
		StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
		return true;
	}
	
	if (code == 0)
	{
		Sleep(500);
		return true;
	}

	if (code > 0) {
		unsigned char buffer[1024] = {0};
		int length = 1024;
		int len = StationTransferClient::GetInstance().RecvData(buffer, length);
		if (len > 0) {
			SerialClient::GetInstance().WriteData((unsigned char *) buffer, len);
			m_logger.WriteInfo("StationTransferClient  recv data and send to serial device. length=[%d]\n", len);
		} else {
			StationTransferClient::GetInstance().Close();
			StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
			Sleep(100);
			return true;
		}
	}

	if (StationTransferClient::GetInstance().TimeOut(600)) {
		m_logger.WriteInfo("StationTransferClient  m_tcpTransfer timeout, now try to reconnect\n");
		StationTransferClient::GetInstance().Close();
		StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
	}

	Sleep(100);
	return true;
}

bool StationTransferClientWorker::ConnectServerFunction() {
	m_logger.WriteInfo("StationTransferClientWorker::ConnectServerFunction.\n");
	m_logger.Flush();
	bool retcode = StationTransferClient::GetInstance().Connect();
	if(retcode) {
		StationTransferClient::GetInstance().State(NTRIP_STATE_CONNECT_SUCCESS);
		m_logger.WriteInfo("StationTransferClientWorker::ConnectServerFunction. retcode=[1]\n");
	} else {
		m_logger.WriteInfo("StationTransferClientWorker::ConnectServerFunction. retcode=[0]\n");
	}
	
	m_logger.Flush();
	Sleep(100);
	return true;
}

bool StationTransferClientWorker::SendAuthorFunction() {
	//连接后的ntrip协议认证
	char authbuffer[256] = {0};
	int length = NtripFormatProtocol::GetInstance().EncodeAuthRequest(Configure::GetInstance().StationMountPoint().c_str(), Configure::GetInstance().StationUsername().c_str(),
		Configure::GetInstance().StationPassword().c_str(), authbuffer, 256);

	//send auth code buffer
	int outLength = StationTransferClient::GetInstance().SendData((unsigned char *) authbuffer, length);

	if(outLength > 0) {
		StationTransferClient::GetInstance().State(NTRIP_STATE_WAIT_AUTHOR_RESPONSE);
	}

	Sleep(100);
	return true;
}

bool StationTransferClientWorker::LoginSuccessFunction() {
	return Process();
}

bool StationTransferClientWorker::WaitAuthorFunction() {
	time_t   now;
	time_t   statetm = StationTransferClient::GetInstance().statetm();
	time(&now);
	if(now - statetm > 30) {
		StationTransferClient::GetInstance().Close();
		StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
	}

	int code = StationTransferClient::GetInstance().IsReadable();

	if(code == -1) {
		Sleep(1000);
		StationTransferClient::GetInstance().Close();
		StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
		return true;
	} 
	
	if (code > 0) {
		int length = 255;
		unsigned char buffer[255] = {0};
		int len = StationTransferClient::GetInstance().RecvData(buffer, length);

		if(len <= 0) {
			return true;
		}

		len = NtripFormatProtocol::GetInstance().DecodeAuthResponse((char *) buffer, len);
		if(len == 0) {
			StationTransferClient::GetInstance().State(NTRIP_STATE_LOGIN_SUCCESS);
		}
	}

	if (StationTransferClient::GetInstance().TimeOut(3)) {
		m_logger.WriteInfo("StationTransferClient  m_tcpTransfer timeout, now try to reconnect\n");
		StationTransferClient::GetInstance().Close();
		StationTransferClient::GetInstance().State(NTRIP_STATE_NO_CONNECT);
	}

	Sleep(100);
	return true;
}



