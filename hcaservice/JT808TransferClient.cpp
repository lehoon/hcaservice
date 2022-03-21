#include "stdafx.h"

#include <time.h>
#include "JT808TransferClient.h"
#include "MessageEnginWorker.h"
#include "SequeNoUtil.h"
#include "Configure.h"
#include "Utils.h"


JT808TransferClient::JT808TransferClient(void)
{
	m_state = STATE_NO_CONNECT;
	std::string logPath = Configure::GetInstance().ModulePath() + "jt808client.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
}

JT808TransferClient::~JT808TransferClient(void)
{
	Close();
	m_state = STATE_NO_CONNECT;
	m_logger.Flush();
}

bool JT808TransferClient::SendMessage(Message * msg) {
	if(msg == NULL || m_state != STATE_LOGIN_SUCCESS ) {
		return false;
	}
    SendData(msg->data, msg->use_len);
	return true;
}

bool JT808TransferClient::Send0x0001Response(unsigned short seqno, unsigned short answerno, unsigned char result) {
	if (m_state != STATE_LOGIN_SUCCESS) return false;
	unsigned char buffer[64] = {0};
	int length = 64;
	int outLen = JT808FormatProtocol::GetInstance().Encode0x0001Response(buffer, length, seqno, answerno, result);
	SendData(buffer, outLen);
	return true;
}

bool JT808TransferClient::SendLocationRequest(PPosition position) {
	//encode package
	Message * msg = NULL;

	if(position->type == REAL_TYPE) {
		int length = PROTOCOL_MESSAGE_0200_LENGTH * 2;
		msg = Utils::GetRawMessage(length);
		msg->type = 0x0200;
		msg->len = length;
		msg->seqno = SequeNoUtil::GetInstance().GetNextSeqNo();
		msg->use_len = JT808FormatProtocol::GetInstance().Encode0200Request(position, msg->data, length, msg->seqno);
	} else {
		int length = PROTOCOL_MESSAGE_0704_LENGTH * 2;
		msg = Utils::GetRawMessage(length);
		msg->type = 0x0704;
		msg->len = length;
		msg->seqno = SequeNoUtil::GetInstance().GetNextSeqNo();
		msg->use_len = JT808FormatProtocol::GetInstance().Encode0704Request(position, msg->data, length, msg->seqno);
	}

	//send data
	SendData(msg->data, msg->use_len);
	time(&(msg->lasttime));
	MessageEnginWorker::GetInstance().PushMessage(msg);
	delete position;
	return true;
}

bool JT808TransferClient::SendRegisterRequest() {
	unsigned char buffer[256] = {0};
	int length = 256;
	unsigned short seqno = SequeNoUtil::GetInstance().GetNextSeqNo();
	int len = JT808FormatProtocol::GetInstance().EncodeRegisterRequest(buffer, length, seqno);

	char hexBuffer[1024] = {0};
	Utils::String2Hex(hexBuffer, (const char *) buffer, len);
	m_logger.WriteDebug("register buffer len=[%d] content=[%s]\n", len, hexBuffer);
	if(len > 0) {
		SendData(buffer, len);
	}

	return true;
}

bool JT808TransferClient::SendAuthorRequest() {
	unsigned char buffer[256] = {0};
	int length = 255;

	unsigned short seqno = SequeNoUtil::GetInstance().GetNextSeqNo();
	int len = JT808FormatProtocol::GetInstance().EncodeAuthorRequest(buffer, length, m_autucode, seqno);
	if(len > 0) SendData(buffer, len);
	return true;
}

bool JT808TransferClient::SendHeartBeatRequest() {
	if (m_state != STATE_LOGIN_SUCCESS) return false;
	unsigned char buffer[64] = {0};
	int length = 64;
	unsigned short seqno = SequeNoUtil::GetInstance().GetNextSeqNo();
	int outLength = JT808FormatProtocol::GetInstance().EncodeHeartBeatRequest(buffer, length, seqno);
	if(outLength > 0) SendData(buffer, outLength);
	return true;
}

void JT808TransferClient::State(Jt808State state) { 
	m_state = state; 
	time(&m_statetm); 
}

JT808TransferClientWorker::JT808TransferClientWorker() 
	: m_recvLength(0)
{
}

JT808TransferClientWorker::~JT808TransferClientWorker() {
	JT808TransferClient::GetInstance().Close();
}

bool JT808TransferClientWorker::InitTask() {
	m_function[0x8001 & 0xfff] = &JT808TransferClientWorker::Handler0x8001Response;
	m_function[0x8100 & 0xfff] = &JT808TransferClientWorker::Handler0x8100Response;

	m_EventMap[STATE_NO_CONNECT] = &JT808TransferClientWorker::ConnectServerFunction;
	m_EventMap[STATE_CONNECT_SUCCESS] = &JT808TransferClientWorker::SendRegisterFunction;
	m_EventMap[STATE_WAIT_REGISTER_RESPONSE] = &JT808TransferClientWorker::WaitRegisterFunction;
	m_EventMap[STATE_REGISTER_SUCCESS] = &JT808TransferClientWorker::SendAuthorFunction;
	m_EventMap[STATE_WAIT_AUTHOR_RESPONSE] = &JT808TransferClientWorker::WaitAuthorFunction;
	m_EventMap[STATE_LOGIN_SUCCESS] = &JT808TransferClientWorker::LoginSuccessFunction;

	memset(m_recvBuffer, 0, JT808CLIENT_BUFFER_SIZE);
	std::string logPath = Configure::GetInstance().ModulePath() + "jt808client-worker.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	return true;
}

bool JT808TransferClientWorker::ExitTask() {
	JT808TransferClient::GetInstance().Close();
	m_logger.Flush();
	return true;
}

bool JT808TransferClientWorker::Work() {
	(this->*m_EventMap[JT808TransferClient::GetInstance().state()])();
	m_logger.Write(1, "jt808 client connection state = [%d]\n", JT808TransferClient::GetInstance().state());
	return true;
}

//注册应答
void JT808TransferClientWorker::Handler0x8100Response(unsigned char * buff, int len, MsgHeader * header) {
	MsgRegisterResponse response;
	response.header = header;
	JT808FormatProtocol::GetInstance().DecodeRegisterResponse(buff, len, response);
	if(response.result == 0) {
		JT808TransferClient::GetInstance().SetAuthCode(response.authcode);
		JT808TransferClient::GetInstance().State(STATE_REGISTER_SUCCESS);
	}
	m_logger.Write(1, "JT808TransferClientWorker::Handler0x8100Response response.result=[%d]\n", response.result);
}

//通用应答
void JT808TransferClientWorker::Handler0x8001Response(unsigned char * buff, int len, MsgHeader * header) {
	MsgPlateResponse response;
	response.header = header;
	JT808FormatProtocol::GetInstance().DecodePlateformResponse(buff, len, response);

	if(response.msgid == 0x0102 && response.result == 0) {
		JT808TransferClient::GetInstance().State(STATE_LOGIN_SUCCESS);
	}

	m_logger.WriteInfo("server response  msgid=[%d] sequeno=[%d] result=[%d]\n", response.msgid, response.reqno, response.result);
	//其他应答回复
	MessageEnginWorker::GetInstance().HandlerResponse(response.msgid, response.reqno);
}

void JT808TransferClientWorker::Handler0x8104Request(unsigned char * buff, int len, MsgHeader *header) {
	m_logger.Write(1, "JT808TransferClientWorker::Handler0x8104Request...\n");
}

void JT808TransferClientWorker::Handler0x8107Request(unsigned char * buff, int len, MsgHeader *header) {
	m_logger.Write(1, "JT808TransferClientWorker::Handler0x8107Request...\n");
}

bool JT808TransferClientWorker::ConnectServerFunction() {
	bool retcode = JT808TransferClient::GetInstance().Connect();
	if(retcode) {
		JT808TransferClient::GetInstance().State(STATE_CONNECT_SUCCESS);
	}

	m_logger.Write(1, "JT808TransferClientWorker::ConnectServerFunction connection code=[%d]\n", retcode);
	Sleep(300);
	return true;
}

bool JT808TransferClientWorker::SendAuthorFunction() {
	bool retcode = JT808TransferClient::GetInstance().SendAuthorRequest();
	if(retcode) {
		JT808TransferClient::GetInstance().State(STATE_WAIT_AUTHOR_RESPONSE);
	}

	m_logger.Write(1, "JT808TransferClientWorker::SendAuthorFunction code=[%d]\n", retcode);
	Sleep(300);
	return true;
}

bool JT808TransferClientWorker::SendRegisterFunction() {
	if(!JT808TransferClient::GetInstance().needregister()) {
		JT808TransferClient::GetInstance().State(STATE_REGISTER_SUCCESS);
		return true;
	}

	bool retcode = JT808TransferClient::GetInstance().SendRegisterRequest();
	if(retcode) {
		JT808TransferClient::GetInstance().State(STATE_WAIT_REGISTER_RESPONSE);
	}

	m_logger.Write(1, "JT808TransferClientWorker::SendRegisterFunction code=[%d]\n", retcode);
	Sleep(300);
	return true;
}

bool JT808TransferClientWorker::WaitEventFunction() {
	m_logger.Write(1, "JT808TransferClientWorker::WaitEventFunction\n");
	return true;
}

bool JT808TransferClientWorker::WaitAuthorFunction() {
	m_logger.Write(1, "JT808TransferClientWorker::WaitAuthorFunction.\n");
	Process();

	if(JT808TransferClient::GetInstance().state() == STATE_LOGIN_SUCCESS) return true;

	time_t   now;
	time_t   statetm = JT808TransferClient::GetInstance().statetm();
	time(&now);
	if(now - statetm > 30) {
		JT808TransferClient::GetInstance().Close();
		JT808TransferClient::GetInstance().State(STATE_NO_CONNECT);
	}

	return true;
}

bool JT808TransferClientWorker::LoginSuccessFunction(){
	m_logger.Write(1, "JT808TransferClientWorker::LoginSuccessFunction.\n");
	return Process();
}

bool JT808TransferClientWorker::WaitRegisterFunction() {
	m_logger.Write(1, "JT808TransferClientWorker::WaitRegisterFunction.\n");
	Process();
	if(JT808TransferClient::GetInstance().state() == STATE_REGISTER_SUCCESS) return true;

	time_t   now;
	time_t   statetm = JT808TransferClient::GetInstance().statetm();
	time(&now);
	if(now - statetm > 30) {
		JT808TransferClient::GetInstance().Close();
		JT808TransferClient::GetInstance().State(STATE_NO_CONNECT);
	}
	return true;
}

bool JT808TransferClientWorker::Process() {
	int code = JT808TransferClient::GetInstance().IsReadable();

	if(code == -1) {
		Sleep(500);
		JT808TransferClient::GetInstance().Close();
		JT808TransferClient::GetInstance().State(STATE_NO_CONNECT);
		m_logger.Write(1, "JT808TransferClientWorker::Process  isreadable code=[%d].\n", code);
		return true;
	}

	if (code == 0)
	{
		m_logger.Write(1, "JT808TransferClientWorker::Process  isreadable code=[%d].\n", code);
		Sleep(500);
		return true;
	}

	int leftLength = 0;
	if(m_recvLength >= JT808CLIENT_BUFFER_SIZE) {
		m_recvLength = 0;
		leftLength = JT808CLIENT_BUFFER_SIZE;
	} else {
		leftLength = JT808CLIENT_BUFFER_SIZE - m_recvLength;
	}

	int recvLength = JT808TransferClient::GetInstance().RecvData(m_recvBuffer + m_recvLength, leftLength);

	if(recvLength < 0) {
		JT808TransferClient::GetInstance().Close();
		JT808TransferClient::GetInstance().State(STATE_NO_CONNECT);
		m_logger.Write(1, "JT808TransferClientWorker::RecvData  recvLength < 0.\n");
		Sleep(300);
		return true;
	}

	if(recvLength == 0) {
		if(JT808TransferClient::GetInstance().TimeOut(5)) {
			JT808TransferClient::GetInstance().Close();
			JT808TransferClient::GetInstance().State(STATE_NO_CONNECT);
			m_logger.Write(1, "JT808TransferClientWorker::RecvData  recvLength == 0.\n");
			Sleep(300);
			return true;
		}
	}

	//当前缓存数据长度
	m_recvLength += recvLength;

	if(m_recvLength <= 15) {
		Sleep(100);
		return true;
	}

	for( ; ; ) {
		int nIndex = 0, nLength = 0;
		int result = JT808FormatProtocol::GetInstance().HandlePackageFrame(m_recvBuffer, m_recvLength, nIndex, nLength);

		if(result == kNeedData) {
			if(nIndex > 0) {
				m_recvLength -= nIndex;
				memmove(m_recvBuffer, m_recvBuffer + nIndex, m_recvLength);
			}
			break;
		}

		if(result == kError){
			m_recvLength = 0;
			break;
		}

		//找到正确的数据包
		unsigned char *pBuffer = (unsigned char *) (m_recvBuffer + nIndex);

		int flag = 0;
		for(int i = 0; i < nLength; i++) {
			if(pBuffer[i] == 0x7d ) flag++;
		}

		int nPkgLength = nLength;
		if(flag > 0) {
			nPkgLength -= JT808FormatProtocol::GetInstance().remote_char_converse_transfer((unsigned char *) pBuffer, nPkgLength);
		}

		char crc = (char) JT808FormatProtocol::GetInstance().Crc8((const unsigned char *) pBuffer, nPkgLength - 2);
		char rawcrc = (char) pBuffer[nPkgLength - 2];

		if(crc != rawcrc) {
			m_recvLength -= nLength;
			memmove(m_recvBuffer, m_recvBuffer + nIndex, m_recvLength);
			continue;
		}

		//找到一个合法的数据包
		MsgHeader header;
		JT808FormatProtocol::GetInstance().DecodeMessageHeader(pBuffer, nPkgLength, &header);
		(this->*m_function[header.msgid  & 0xfff])(pBuffer, nPkgLength, &header);
		//action buffer length and data
		m_recvLength -= nLength;
		memmove(m_recvBuffer, m_recvBuffer + nIndex, m_recvLength);
		continue;
	}

	//处理数据
	Sleep(100);
	return true;
}