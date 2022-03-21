#include "stdafx.h"

#include "SqliteStoreClient.h"
#include "JT808TransferClient.h"
#include "StationTransferClient.h"
#include "MessageLocationWorker.h"
#include "Configure.h"
#include "Utils.h"

MessageLocationWorker::MessageLocationWorker(void) 
	: m_posCount(0)
{
}

MessageLocationWorker::~MessageLocationWorker(void)
{
}

bool MessageLocationWorker::InitTask() {
	std::string logPath = Configure::GetInstance().ModulePath() + "location-worker.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	return true;
}

bool MessageLocationWorker::ExitTask() {
	m_logger.WriteInfo("MessageLocationWorker::ExitTask().\n");
	m_logger.Flush();
	while (m_posCount > 0) {
		PPosition msg = PopPosition();
		if(msg == NULL) break;
		JT808TransferClient::GetInstance().SendLocationRequest(msg);
		delete msg;
	}
	return true;
}

bool MessageLocationWorker::Work() {
	//m_logger.WriteInfo("MessageLocationWorker::Work()   msgcount=[%d]\n", m_posCount);
	if(m_posCount == 0) {
		if(!JT808TransferClient::GetInstance().IsLogin()){
			Sleep(300);
			return true;
		}

		if(JT808TransferClient::GetInstance().SendTimeOut(15)) {
			JT808TransferClient::GetInstance().SendHeartBeatRequest();
		}
		SqliteStoreClient::GetInstance().LoadStorePosition();
		Sleep(100);
		return true;
	}

	PPosition msg = PopPosition();
	if(msg == NULL) {
		Sleep(100);
		return true;
	}

	//当前没有连接到平台,需要将已经缓存的数据都录入sqlite当做历史数据
	if(!JT808TransferClient::GetInstance().IsLogin()) {
		ChangeMessageHisState();
		m_logger.WriteInfo("808 server now not connection,so save location [%02d%02d%02d%02d%02d%02d]data to sqlite db.\n", msg->year, msg->month, msg->day, msg->hour, msg->minute, msg->second);
		SqliteStoreClient::GetInstance().InsertData(msg);
		delete msg;
		Sleep(10);
		return true;
	}

	m_logger.WriteInfo("send location [%02d%02d%02d%02d%02d%02d] [%d,%d,%d] data to remote server.\n", msg->year, msg->month, msg->day, msg->hour, msg->minute, msg->second, msg->state, msg->diff, msg->starnum);
	JT808TransferClient::GetInstance().SendLocationRequest(msg);
	Sleep(100);
	return true;
}

void MessageLocationWorker::PushPosition(Position * msg) {
	InterlockedIntrinsicLock::Lock(&m_posLock);
	if(m_posCount > 36000) {
		Position * msg = m_posList.front();
		m_posList.pop_front();
		delete msg;
		m_posCount--;
	}
	m_posList.push_back(msg);
	m_posCount++;
	InterlockedIntrinsicLock::Unlock(&m_posLock);
}

Position * MessageLocationWorker::PopPosition() {
	if(m_posCount == 0) return NULL;
	InterlockedIntrinsicLock::Lock(&m_posLock);
	Position * msg = m_posList.front();
	m_posList.pop_front();
	m_posCount--;
	InterlockedIntrinsicLock::Unlock(&m_posLock);
	return msg;
}

unsigned int MessageLocationWorker::PosCount() {
	/*unsigned int count = 0;
	InterlockedIntrinsicLock::Lock(&m_posLock);
	count = m_posCount;
	InterlockedIntrinsicLock::Unlock(&m_posLock);*/
	return m_posCount;
}

void MessageLocationWorker::SendPostion(unsigned char * buffer, int & length) {
	StationTransferClient::GetInstance().SendRealTimeLocation(buffer, length);
}

void MessageLocationWorker::ChangeMessageHisState() {
	if(m_posCount == 0) return;
	InterlockedIntrinsicLock::Lock(&m_posLock);
	std::list<PPosition>::iterator iter = m_posList.begin();

	for(; iter != m_posList.end(); iter++) {
		(*iter)->type = OLD_TYPE;
	}

	InterlockedIntrinsicLock::Unlock(&m_posLock);
}
