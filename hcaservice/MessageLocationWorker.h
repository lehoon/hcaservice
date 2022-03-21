#pragma once

#include <list>

#include "base_type.h"
#include "Lock.h"
#include "Logger.h"
#include "IWorkThread.h"
#include "Singleton.h"


class MessageLocationWorker : public IThreadUser, public Singleton<MessageLocationWorker>
{
public:
	MessageLocationWorker(void);
	~MessageLocationWorker(void);

public:
	void PushPosition(Position * msg);
	Position * PopPosition();
	unsigned int PosCount();
	void SendPostion(unsigned char * buffer, int & length);

private:
	void ChangeMessageHisState();

public:
	bool InitTask();
	bool ExitTask();
	bool Work();

private:
	unsigned int m_posCount;
	std::list<Position *> m_posList;
	InterlockedIntrinsicLock::LOCK m_posLock;
	Logger		m_logger;
};

