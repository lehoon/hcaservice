#ifndef _HCASERVICE_MESSAGEENGIN_WORKER_H_
#define _HCASERVICE_MESSAGEENGIN_WORKER_H_

#include <list>

#include "base_type.h"
#include "Lock.h"
#include "Logger.h"
#include "IWorkThread.h"
#include "Singleton.h"

template<unsigned short msgid, unsigned short seqno>
class MessageComp {
public:
	bool operator() (Message * msg) {
		return (msg->type == msgid && msg->seqno == seqno);
	}
};

class MessageEnginWorker : public IThreadUser, public Singleton<MessageEnginWorker>
{
public:
	MessageEnginWorker(void);
	~MessageEnginWorker(void);

public:
	bool InitTask();
	bool ExitTask();
	bool Work();

public:
	void PushMessage(Message * msg);
	Message * PopMessage();
	unsigned int MsgCount();
	void HandlerResponse(unsigned short msgid, unsigned short seqno);

private:
	unsigned int m_msgCount;
	std::list<Message *> m_msgList;
	InterlockedIntrinsicLock::LOCK m_msgLock;

	Logger               m_logger;
};

#endif /*_HCASERVICE_MESSAGEENGIN_WORKER_H_*/
