#include "stdafx.h"

#include <process.h>

#include "IWorkThread.h"

IThreadUser::IThreadUser(void)
{
}

IThreadUser::~IThreadUser(void)
{
}

IThreadWrap::IThreadWrap(void)
{
	threadHandle_ = (NULL);
	threadId_ = (0);
	quitEt_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

IThreadWrap::~IThreadWrap(void)
{
	if (NULL != quitEt_) {
		::CloseHandle(quitEt_);
		quitEt_ = NULL;
	}
}

IWorkThread::IWorkThread(void)
{
}

IWorkThread::~IWorkThread(void)
{
	StopThread();
}

IWorkThread::IWorkThread(IThreadUser* pWorker) {
	threadHandle_ = NULL;
	threadId_ = 0;
	worker_ = pWorker;
}


bool IWorkThread::StartThread()
{
	if (!worker_->InitTask()) {
		return false;
	}

	threadHandle_ = (HANDLE)_beginthreadex(NULL, 0, IWorkThread::threadFunc, (void*)this, 0, &threadId_);
	return threadHandle_ != 0;
}
bool IWorkThread::StopThread()
{
	if (NULL != threadHandle_) {
		::SetEvent(quitEt_);
		WaitForSingleObject(threadHandle_, INFINITE);
		CloseHandle(threadHandle_);
		threadHandle_ = NULL;
	}
	return TRUE;
}

unsigned int IWorkThread::threadFunc(void* p)
{
	IWorkThread* threadObj = (IWorkThread*)p;
	return threadObj->DoWork();
}

unsigned int IWorkThread::DoWork()
{
	HANDLE hEvts[1];
	hEvts[0] = quitEt_;
	while (TRUE) 
	{
		DWORD dwRet = WaitForMultipleObjects(1, hEvts, FALSE, 0);
		dwRet -= WAIT_OBJECT_0;
		if (dwRet == 0) 
		{
			break;
		}

		if (!worker_->Work()) 
		{
			break;//quit thread
		}
	}

	worker_->ExitTask();
	_endthreadex(0);
	return 0;
}

