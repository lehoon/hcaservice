#ifndef _HCASERVICE_LOCKER_H_
#define _HCASERVICE_LOCKER_H_

#include <windows.h>

class ILock  
{  
public:  
	virtual ~ILock() {}  

	virtual void Lock() const = 0;  
	virtual void Unlock() const = 0;
	virtual bool LockNoWait() const {
		return false;
	}
};  


class Mutex : public ILock  
{  
public:  
	Mutex();  
	~Mutex();  

	virtual void Lock() const;  
	virtual void Unlock() const;  

private:  
	HANDLE m_mutex;  
};  

class CriSection : public ILock  
{  
public:  
	CriSection();  
	~CriSection();  

	virtual void Lock() const;  
	virtual void Unlock() const;  
	virtual bool LockNoWait() const;

private:  
	CRITICAL_SECTION m_critclSection;  
};  

namespace InterlockedIntrinsicLock
{
	typedef unsigned LOCK, *PLOCK;

	enum { LOCK_IS_FREE = 0, LOCK_IS_TAKEN = 1 };

	void Lock(PLOCK pl);
	void Unlock(PLOCK pl);
	bool LockNoWait(PLOCK pl);
}

#endif /*_HCASERVICE_LOCKER_H_*/
