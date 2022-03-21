#include "stdafx.h"

#include "SequeNoUtil.h"

unsigned short SequeNoUtil::m_seqno = 0;

SequeNoUtil::SequeNoUtil(void)
{
}

SequeNoUtil::~SequeNoUtil(void)
{
}

unsigned short SequeNoUtil::GetNextSeqNo() {
	InterlockedIntrinsicLock::Lock(&m_lock);
	if(m_seqno > 65535) {
		m_seqno = 0;
	}
	InterlockedIntrinsicLock::Unlock(&m_lock);
	return m_seqno++;
}