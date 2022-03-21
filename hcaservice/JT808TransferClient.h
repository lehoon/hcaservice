#ifndef _HCASERVICE_JT808TRANSFERCLIENT_H
#define _HCASERVICE_JT808TRANSFERCLIENT_H

#include "base_type.h"
#include "IWorkThread.h"
#include "TcpTransferBase.h"
#include "Singleton.h"
#include "Logger.h"
#include "JT808FormatProtocol.h"

#define  JT808CLIENT_BUFFER_SIZE  2048

enum Jt808State{
	STATE_NO_CONNECT = 0,
	STATE_CONNECT_SUCCESS,
	STATE_WAIT_REGISTER_RESPONSE,
	STATE_REGISTER_SUCCESS,
	STATE_WAIT_AUTHOR_RESPONSE,
	STATE_LOGIN_SUCCESS
};

class JT808TransferClient : public TcpTransferBase, public Singleton<JT808TransferClient> 
{
public:
	JT808TransferClient(void);
	~JT808TransferClient(void);

public:
	bool SendMessage(Message * msg);
	//协议编码，并发送出去
	bool Send0x0002Request(Message * msg);
	bool Send0x0100Request(Message * msg);
	bool Send0x0200Request(Message * msg);
	bool Send0x0704Request(Message * msg);

	bool SendLocationRequest(PPosition msg);
	//是否登录平台
	bool IsLogin() { return m_state == STATE_LOGIN_SUCCESS; }
	void SetAuthCode(std::string & autocode) { m_autucode = autocode; }
	int  state() { return m_state; }
	time_t statetm() {return m_statetm; }
	void State(Jt808State state);
	void SetNeedRegister(bool need) { m_needregister = need; }
	bool needregister() { return m_needregister;}
public:
	bool SendHeartBeatRequest();
	bool Send0x0001Response(unsigned short seqno, unsigned short answerno, unsigned char result);
	bool SendAuthorRequest();
	bool SendRegisterRequest();

private:
	char  * m_phone;           //手机号
	bool    m_needregister;    //是否需要注册

	Jt808State m_state;		   //state
	time_t     m_statetm;      //modify time
	std::string  m_autucode;   //鉴权码
	Logger       m_logger;
};

class JT808TransferClientWorker;
typedef void (JT808TransferClientWorker::*HandleFunction)(unsigned char * buff, int len, MsgHeader *header);

class JT808TransferClientWorker : public IThreadUser
{
	typedef bool (JT808TransferClientWorker::*EventFunction)();
public:
	JT808TransferClientWorker(void);
	~JT808TransferClientWorker(void);

public:
	void Handler0x8001Response(unsigned char * buff, int len, MsgHeader *header);
	void Handler0x8100Response(unsigned char * buff, int len, MsgHeader *header);
	void Handler0x8104Request(unsigned char * buff, int len, MsgHeader *header);
	void Handler0x8107Request(unsigned char * buff, int len, MsgHeader *header);

public:
	bool ConnectServerFunction();
	bool SendAuthorFunction();
	bool SendRegisterFunction();
	bool WaitEventFunction();
	bool WaitRegisterFunction();
	bool WaitAuthorFunction();
	bool LoginSuccessFunction();

public:
	bool Process();

public:
	bool InitTask();
	bool ExitTask();
	bool Work();

private:
	unsigned char        m_recvBuffer[JT808CLIENT_BUFFER_SIZE];
	unsigned int         m_recvLength;
	HandleFunction       m_function[0xfff];

	//client state and action map
	EventFunction	     m_EventMap[0x8];
	Logger               m_logger;
};

#endif /*_HCASERVICE_JT808TRANSFERCLIENT_H*/
