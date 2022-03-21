#ifndef _HCASERVICE_SERIALPORT_WORKER_H_
#define _HCASERVICE_SERIALPORT_WORKER_H_

#include "stdafx.h"

enum SERIALPORT_STATE {
	SERIAL_STATE_NO_CONNECT = 0,
	SERIAL_STATE_CONNECT_SUCCESS
};

class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);
public:
	bool InitPort( UINT  portNo,UINT  baud,char  parity,UINT  databits, UINT  stopsbits,DWORD dwCommEvents);
	bool InitPort( UINT  portNo ,const LPDCB& plDCB );
	bool WriteData(unsigned char* pData, unsigned int length);
	UINT GetBytesInCOM();
	bool ReadChar(char &cRecved);
	void ClosePort();
private:
	bool openPort( UINT  portNo );
private:
	HANDLE  m_hComm;
	CRITICAL_SECTION m_csCommunicationSync;

protected:
	SERIALPORT_STATE    m_state;
};
#endif /*_HCASERVICE_SERIALPORT_WORKER_H_*/
