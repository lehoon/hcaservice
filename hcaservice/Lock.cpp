#include "stdafx.h"
#include "Lock.h"


Mutex::Mutex()
{
	m_mutex = ::CreateMutex(NULL, FALSE, NULL);
}
Mutex::~Mutex()
{
	::CloseHandle(m_mutex);
}
void Mutex::Lock() const
{
	DWORD d = WaitForSingleObject(m_mutex, INFINITE);
}
void Mutex::Unlock() const
{
	::ReleaseMutex(m_mutex);
}
//---------------------------------------------------------------------------   
CriSection::CriSection()
{
	::InitializeCriticalSection(&m_critclSection);
}
CriSection::~CriSection()
{
	::DeleteCriticalSection(&m_critclSection);
}
void CriSection::Lock() const
{
	::EnterCriticalSection((LPCRITICAL_SECTION)&m_critclSection);
}
void CriSection::Unlock() const  
{  
	::LeaveCriticalSection((LPCRITICAL_SECTION)&m_critclSection);
}
bool CriSection::LockNoWait() const
{
	return ::TryEnterCriticalSection((LPCRITICAL_SECTION)&m_critclSection) == TRUE;
}
namespace InterlockedIntrinsicLock
{
	void Lock(PLOCK pl)
	{
		while (InterlockedCompareExchange((long *)pl,
			LOCK_IS_TAKEN, // exchange
			LOCK_IS_FREE)  // comparand
			== LOCK_IS_TAKEN)
		{
			// spin!
			// call __yield() here on the IPF architecture to improve performance.
		}
	}

	void Unlock(PLOCK pl) 
	{
		InterlockedExchange((long *)pl, LOCK_IS_FREE);
	}

	bool LockNoWait(PLOCK pl)
	{
		return InterlockedCompareExchange((long *)pl,
			LOCK_IS_TAKEN, // exchange
			LOCK_IS_FREE)  // comparand
			== LOCK_IS_FREE;
	}
}