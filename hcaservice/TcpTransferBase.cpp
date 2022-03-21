#include "stdafx.h"
#include <time.h>

#include "TcpTransferBase.h"


TcpTransferBase::TcpTransferBase(void)
{
}


TcpTransferBase::~TcpTransferBase(void)
{
	Close();
}

TcpTransferBase::TcpTransferBase(const char * host, const int port)
	: m_port(port)
	, m_second(0)
	, m_musec(10000)
{
	if(host != NULL || strlen(host) > 0) {
		strncpy_s(m_host, host, 31);
	}
}

TcpTransferBase::TcpTransferBase(const char * host, const int port, unsigned int second, unsigned int usec) 
	: m_port(port)
	, m_second(second)
	, m_musec(usec)
{
	if(host != NULL || strlen(host) > 0) {
		strncpy_s(m_host, host, 31);
	}
}

bool TcpTransferBase::InitSocketEnv() {
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	int error = WSAStartup(wVersionRequested, &wsaData);

	if(error != 0) {
		return false;
	}

	if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return false;
	}

	return true;
}

bool TcpTransferBase::Connect() {
	if(m_host == NULL || strlen(m_host) == 0) {
		return false;
	}

	if(m_port <= 0 || m_port > 65535) {
		return false;
	}

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_socket == INVALID_SOCKET) return false;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_host);
	addr.sin_port = htons(m_port);

	int result = ::connect(m_socket, (sockaddr *) & addr, sizeof(sockaddr));
	if(result != 0) {
		::closesocket(m_socket);
		m_socket = NULL;
		return false;
	}

	SetNoBlockMode();
	time(&m_recvtime);
	return true;
}

fd_set TcpTransferBase::FdSet() {
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(m_socket, &fs);
	return fs;
}

void TcpTransferBase::SetNoBlockMode() {
	unsigned long mode = 1;
	ioctlsocket(m_socket, FIONBIO, &mode);

	char flag = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

bool TcpTransferBase::ReConnectSocket() {
	Close();
	::Sleep(1000);
	return Connect();
}

int TcpTransferBase::SendData(unsigned char * buffer, int length) {
	if(m_socket == NULL) return 0;
	time(&m_sendtime);
	return ::send(m_socket, (const char *) buffer, length, 0);
}

int TcpTransferBase::RecvData(unsigned char * buffer, int &length) {
	int recvnum = recv(m_socket, (char *) buffer, length, 0);
	if(recvnum > 0) {
		time(&m_recvtime);
	}
	return recvnum;
}

bool TcpTransferBase::TimeOut(unsigned int timeout) {
	time_t now;
	time(&now);

	if((now - m_recvtime) > timeout) {
		return true;
	}
	return false;
}

bool TcpTransferBase::SendTimeOut(unsigned int timeout) {
	time_t now;
	time(&now);

	if(now - m_sendtime > timeout) {
		return true;
	}

	return false;
}

int TcpTransferBase::IsReadable() {
	fd_set fs;
	FD_ZERO(&fs);
	FD_SET(m_socket, &fs);

	timeval timeout; 
	timeout.tv_sec = m_second; 
	timeout.tv_usec = m_musec; 

	int retcode = select(m_socket + 1, &fs, NULL, NULL, &timeout);
	if(retcode < 0) {
		return -1;
	}

	return FD_ISSET(m_socket, &fs);
}

bool TcpTransferBase::Close() {
	if(m_socket != NULL) {
		closesocket(m_socket);
		m_socket = NULL;
	}

	return true;
}

void TcpTransferBase::SetHost(const char * addr) {
	if(addr != NULL || strlen(addr) > 0) {
		strncpy_s(m_host, addr, 31);
	}
}

void TcpTransferBase::SetPort(const int port) {
	m_port = port;
}

void TcpTransferBase::SetTimeOut(unsigned int second, unsigned int usec) {
	m_second = second;
	m_musec = usec;
}
