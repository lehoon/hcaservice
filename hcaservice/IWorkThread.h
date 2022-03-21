#ifndef _HCASERVICE_WORKTHREAD_H
#define _HCASERVICE_WORKTHREAD_H

#include "stdafx.h"

class IThreadUser;
class IThreadWrap
{
public:
	IThreadWrap();
	virtual ~IThreadWrap();
	virtual bool StartThread()=0;
	virtual bool StopThread()=0;

protected:
	HANDLE quitEt_;
	HANDLE threadHandle_;
	unsigned int threadId_;
};

class IWorkThread : public IThreadWrap
{
public:
	IWorkThread(void);
	IWorkThread(IThreadUser* pWorker);
	~IWorkThread(void);

public:
	virtual bool StartThread();
	virtual bool StopThread();

	static unsigned int WINAPI threadFunc(void* p);
	unsigned int DoWork();

private:
	IThreadUser* worker_;
};

class IThreadUser
{
public:
	IThreadUser(void);
	~IThreadUser(void);

public:
	virtual bool InitTask() = 0;
	virtual bool ExitTask() = 0;
	virtual bool Work() = 0;
};

#endif /*_HCASERVICE_WORKTHREAD_H*/
