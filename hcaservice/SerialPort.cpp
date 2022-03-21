#include "stdafx.h"
#include "SerialPort.h"
#include <process.h>
#include <memory>

CSerialPort::CSerialPort(void) 
	: m_state(SERIAL_STATE_NO_CONNECT)
	, m_hComm(INVALID_HANDLE_VALUE)
{
    InitializeCriticalSection(&m_csCommunicationSync);
}

CSerialPort::~CSerialPort(void)
{
	ClosePort();
}

bool CSerialPort::InitPort( UINT portNo /*= 1*/,UINT baud /*= CBR_9600*/,char parity /*= 'N'*/,
                            UINT databits /*= 8*/, UINT stopsbits /*= 1*/,DWORD dwCommEvents /*= EV_RXCHAR*/ )
{
    /** ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ���DCB�ṹ */
    char szDCBparam[50];
    sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

    /** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
    if (!openPort(portNo))
    {
        return false;
    }

    /** �����ٽ�� */
    EnterCriticalSection(&m_csCommunicationSync);

    /** �Ƿ��д����� */
    BOOL bIsSuccess = TRUE;

    /** �ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
     *  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
     */
    /*if (bIsSuccess )
    {
        bIsSuccess = SetupComm(m_hComm,10,10);
    }*/

    /** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
    COMMTIMEOUTS  CommTimeouts;
    CommTimeouts.ReadIntervalTimeout         = 0;
    CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
    CommTimeouts.ReadTotalTimeoutConstant    = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant   = 0;
    if (bIsSuccess) {
        bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
    }

    DCB  dcb;
    if ( bIsSuccess )
    {
        // ��ANSI�ַ���ת��ΪUNICODE�ַ���
        DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, NULL, 0);
        wchar_t *pwText = new wchar_t[dwNum] ;
        if (!MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
        {
            bIsSuccess = TRUE;
        }

        /** ��ȡ��ǰ�������ò���,���ҹ��촮��DCB���� */
        bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		int error = GetLastError();
        /** ����RTS flow���� */
        dcb.fRtsControl = RTS_CONTROL_ENABLE;

        /** �ͷ��ڴ�ռ� */
        delete [] pwText;
    }

    if ( bIsSuccess )
    {
        /** ʹ��DCB�������ô���״̬ */
        bIsSuccess = SetCommState(m_hComm, &dcb);
    }

    /**  ��մ��ڻ����� */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    LeaveCriticalSection(&m_csCommunicationSync);
	if(bIsSuccess == TRUE) {
		m_state = SERIAL_STATE_CONNECT_SUCCESS;
		return true;
	}

    return false;
}

bool CSerialPort::InitPort( UINT portNo ,const LPDCB& plDCB )
{
    /** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
    if (!openPort(portNo))
    {
        return false;
    }

    /** �����ٽ�� */
    EnterCriticalSection(&m_csCommunicationSync);

    /** ���ô��ڲ��� */
    if (!SetCommState(m_hComm, plDCB))
    {
        return false;
    }

    /**  ��մ��ڻ����� */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    /** �뿪�ٽ�� */
    LeaveCriticalSection(&m_csCommunicationSync);
    return true;
}

void CSerialPort::ClosePort()
{
    /** ����д��ڱ��򿪣��ر��� */
    if( m_hComm != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hComm );
        m_hComm = INVALID_HANDLE_VALUE;
    }

	m_state = SERIAL_STATE_NO_CONNECT;
}

bool CSerialPort::openPort( UINT portNo )
{
    /** �����ٽ�� */
    EnterCriticalSection(&m_csCommunicationSync);

    /** �Ѵ��ڵı��ת��Ϊ�豸�� */
    char szPort[50];
    sprintf_s(szPort, "\\\\.\\COM%d", portNo);

    /** ��ָ���Ĵ��� */
    m_hComm = CreateFileA(szPort,  /** �豸��,COM1,COM2�� */
              GENERIC_READ | GENERIC_WRITE, /** ����ģʽ,��ͬʱ��д */  
              0,                            /** ����ģʽ,0��ʾ������ */
              NULL,                         /** ��ȫ������,һ��ʹ��NULL */
              OPEN_EXISTING,                /** �ò�����ʾ�豸�������,���򴴽�ʧ�� */
              0,
              0);

    /** �����ʧ�ܣ��ͷ���Դ������ */
    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        LeaveCriticalSection(&m_csCommunicationSync);
        return false;
    }

    /** �˳��ٽ��� */
    LeaveCriticalSection(&m_csCommunicationSync);
    return true;
}

UINT CSerialPort::GetBytesInCOM()
{
    DWORD dwError = 0;
    COMSTAT  comstat;
    memset(&comstat, 0, sizeof(COMSTAT));

    UINT BytesInQue = 0;
    if ( ClearCommError(m_hComm, &dwError, &comstat) ) {
        BytesInQue = comstat.cbInQue;
	}
    return BytesInQue;
}

bool CSerialPort::ReadChar( char &cRecved )
{
    BOOL  bResult     = TRUE;
    DWORD BytesRead   = 0;
    if(m_hComm == INVALID_HANDLE_VALUE) {
        return false;
    }

    EnterCriticalSection(&m_csCommunicationSync);
    bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
    if ((!bResult))
    {
        DWORD dwError = GetLastError();
        PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
        LeaveCriticalSection(&m_csCommunicationSync);
        return false;
    }

    LeaveCriticalSection(&m_csCommunicationSync);
    return (BytesRead == 1);
}

bool CSerialPort::WriteData( unsigned char* pData, unsigned int length)
{
    BOOL   bResult = TRUE;
    DWORD  BytesToSend = 0;
    if(m_hComm == INVALID_HANDLE_VALUE) {
        return false;
    }

    EnterCriticalSection(&m_csCommunicationSync);
    bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
    if (!bResult) {
        DWORD dwError = GetLastError();
        PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
        LeaveCriticalSection(&m_csCommunicationSync);
        return false;
    }

    LeaveCriticalSection(&m_csCommunicationSync);
    return true;
}