#ifndef _HCASERVICE_TCPTRANSFER_BASE_H_
#define _HCASERVICE_TCPTRANSFER_BASE_H_

#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

class TcpTransferBase
{
public:
	TcpTransferBase(void);
	~TcpTransferBase(void);
	TcpTransferBase(const char * host, const int port);
	TcpTransferBase(const char * host, const int port, unsigned int second, unsigned int usec);

public:
	static bool InitSocketEnv();
	bool Connect();
	bool Close();
	fd_set FdSet();
	void SetNoBlockMode();
	bool ReConnectSocket();
	int IsReadable();
	bool TimeOut(unsigned int timeout);
	int SendData(unsigned char * buffer, int length);
	int RecvData(unsigned char * buffer, int &length);

	void SetHost(const char * addr);
	void SetPort(const int port);
	void SetTimeOut(unsigned int second, unsigned int usec);

	time_t SendTime() { return m_sendtime; }
	bool SendTimeOut(unsigned int timeout);
private:
	int m_port;
	char m_host[32];
	SOCKET m_socket;

	time_t m_recvtime;
	time_t m_sendtime;

	unsigned int m_second;   //second
	unsigned int m_musec;    //usec
};

#endif /*_HCASERVICE_TCPTRANSFER_BASE_H_*/