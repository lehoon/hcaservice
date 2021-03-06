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
    /** 临时变量,将制定参数转化为字符串形式,以构造DCB结构 */
    char szDCBparam[50];
    sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

    /** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
    if (!openPort(portNo))
    {
        return false;
    }

    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 是否有错误发生 */
    BOOL bIsSuccess = TRUE;

    /** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
     *  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
     */
    /*if (bIsSuccess )
    {
        bIsSuccess = SetupComm(m_hComm,10,10);
    }*/

    /** 设置串口的超时时间,均设为0,表示不使用超时限制 */
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
        // 将ANSI字符串转换为UNICODE字符串
        DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, NULL, 0);
        wchar_t *pwText = new wchar_t[dwNum] ;
        if (!MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
        {
            bIsSuccess = TRUE;
        }

        /** 获取当前串口配置参数,并且构造串口DCB参数 */
        bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		int error = GetLastError();
        /** 开启RTS flow控制 */
        dcb.fRtsControl = RTS_CONTROL_ENABLE;

        /** 释放内存空间 */
        delete [] pwText;
    }

    if ( bIsSuccess )
    {
        /** 使用DCB参数配置串口状态 */
        bIsSuccess = SetCommState(m_hComm, &dcb);
    }

    /**  清空串口缓冲区 */
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
    /** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
    if (!openPort(portNo))
    {
        return false;
    }

    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 配置串口参数 */
    if (!SetCommState(m_hComm, plDCB))
    {
        return false;
    }

    /**  清空串口缓冲区 */
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    /** 离开临界段 */
    LeaveCriticalSection(&m_csCommunicationSync);
    return true;
}

void CSerialPort::ClosePort()
{
    /** 如果有串口被打开，关闭它 */
    if( m_hComm != INVALID_HANDLE_VALUE )
    {
        CloseHandle( m_hComm );
        m_hComm = INVALID_HANDLE_VALUE;
    }

	m_state = SERIAL_STATE_NO_CONNECT;
}

bool CSerialPort::openPort( UINT portNo )
{
    /** 进入临界段 */
    EnterCriticalSection(&m_csCommunicationSync);

    /** 把串口的编号转换为设备名 */
    char szPort[50];
    sprintf_s(szPort, "\\\\.\\COM%d", portNo);

    /** 打开指定的串口 */
    m_hComm = CreateFileA(szPort,  /** 设备名,COM1,COM2等 */
              GENERIC_READ | GENERIC_WRITE, /** 访问模式,可同时读写 */  
              0,                            /** 共享模式,0表示不共享 */
              NULL,                         /** 安全性设置,一般使用NULL */
              OPEN_EXISTING,                /** 该参数表示设备必须存在,否则创建失败 */
              0,
              0);

    /** 如果打开失败，释放资源并返回 */
    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        LeaveCriticalSection(&m_csCommunicationSync);
        return false;
    }

    /** 退出临界区 */
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