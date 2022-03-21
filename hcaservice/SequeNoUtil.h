#ifndef _HCASERVICE_SEQUENO_UTIL_H_
#define _HCASERVICE_SEQUENO_UTIL_H_

#include "Singleton.h"
#include "Lock.h"

class SequeNoUtil : public Singleton<SequeNoUtil>
{
public:
	SequeNoUtil(void);
	~SequeNoUtil(void);

public:
	unsigned short GetNextSeqNo();

private:
	static unsigned short m_seqno;
	InterlockedIntrinsicLock::LOCK m_lock;
};

#endif /*_HCASERVICE_SEQUENO_UTIL_H_*/