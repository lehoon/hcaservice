#include "stdafx.h"

#include <time.h>

#include "MessageEnginWorker.h"
#include "SqliteStoreClient.h"
#include "JT808TransferClient.h"
#include "Configure.h"
#include "Utils.h"

MessageEnginWorker::MessageEnginWorker(void) 
	:m_msgCount(0)
{
}

MessageEnginWorker::~MessageEnginWorker(void)
{
}

bool MessageEnginWorker::InitTask() {
	std::string logPath = Configure::GetInstance().ModulePath() + "msgengin-worker.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	return true;
}

bool MessageEnginWorker::ExitTask() {
	m_logger.WriteInfo("MessageEnginWorker::ExitTask.\n");
	m_logger.Flush();
	while (MsgCount() > 0) {
		Message * msg = PopMessage();
		if(msg == NULL) break;
		JT808TransferClient::GetInstance().SendMessage(msg);
		Utils::FreeRawMessage(msg);
	}	
	return true;
}

bool MessageEnginWorker::Work() {
	int msgcount = MsgCount();
	//m_logger.WriteInfo("MessageEnginWorker::Work()   msgcount=[%d]\n", msgcount);

	if(msgcount == 0) {
		Sleep(100);
		return true;
	}

	std::list<Message*> timeoutList;
	time_t now;
	time(&now);
	//do action and repeat send data
	InterlockedIntrinsicLock::Lock(&m_msgLock);
	std::list<Message*>::iterator iter = m_msgList.begin();
	std::list<Message*>::iterator end = m_msgList.end();
	for(; iter != end; ) {
		Message *msg = *iter;
		if(now - msg->lasttime > 3) {
			m_msgList.erase(iter++);
			msg->timeout++;
			m_msgCount--;
			timeoutList.push_back(msg);
		} else {
			break;
		}
	}
	InterlockedIntrinsicLock::Unlock(&m_msgLock);

	m_logger.WriteInfo("timeoutList.size()=[%d].\n", timeoutList.size());
	if(timeoutList.size() == 0) {
		Sleep(1000);
		return true;
	}

	for(iter = timeoutList.begin(); iter != timeoutList.end(); ) {
		Message *msg = *iter;

		if(msg->timeout > 3) {
			delete msg;
		} else {
			PushMessage(msg);
			JT808TransferClient::GetInstance().SendMessage(msg);
		}

		timeoutList.erase(iter++);
	}

	timeoutList.clear();
	Sleep(1000);
	return true;
}

void MessageEnginWorker::HandlerResponse(unsigned short msgid, unsigned short seqno) {
	InterlockedIntrinsicLock::Lock(&m_msgLock);
	std::list<Message*>::iterator iter = m_msgList.begin();
	std::list<Message*>::iterator end = m_msgList.end();
	for(; iter != end; ) {
		if((*iter)->seqno == seqno && (*iter)->type == msgid) {
			Utils::FreeRawMessage(*iter);
			m_msgCount--;
			m_msgList.erase(iter++);
			break;
		}
		iter++;
	}
	InterlockedIntrinsicLock::Unlock(&m_msgLock);
}

unsigned int MessageEnginWorker::MsgCount(){
	/*unsigned int count = 0;
	InterlockedIntrinsicLock::Lock(&m_msgLock);
	count = m_msgCount;
	InterlockedIntrinsicLock::Unlock(&m_msgLock);*/
	return m_msgCount;
}

void MessageEnginWorker::PushMessage(Message * msg) {
	Utils::FreeRawMessage(msg);
	return;

	InterlockedIntrinsicLock::Lock(&m_msgLock);

	if(m_msgCount > 36000) {
		Message * m = m_msgList.front();
		delete m;
		m_msgList.pop_front();
		m_msgCount--;
	}

	m_msgList.push_back(msg);
	m_msgCount++;
	InterlockedIntrinsicLock::Unlock(&m_msgLock);
	return;
}

Message * MessageEnginWorker::PopMessage() {
	if(m_msgCount == 0) return NULL;
	InterlockedIntrinsicLock::Lock(&m_msgLock);
	Message * msg = m_msgList.front();
	m_msgList.pop_front();
	m_msgCount--;
	InterlockedIntrinsicLock::Unlock(&m_msgLock);
	return msg;
}



