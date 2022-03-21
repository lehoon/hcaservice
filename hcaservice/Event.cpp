#include "stdafx.h"
#include "Event.h"


Event::Event(BOOL bInitiallyOwn, BOOL bManualReset, LPCTSTR pstrName,
	LPSECURITY_ATTRIBUTES lpsaAttribute)
{
	m_hObject = ::CreateEvent(lpsaAttribute, bManualReset,
		bInitiallyOwn, pstrName);
}

Event::~Event()
{
	if (NULL != m_hObject) {
		::CloseHandle(m_hObject);
		m_hObject = NULL;
	}
}

BOOL Event::SetEvent()
{
	return ::SetEvent(m_hObject);
}
BOOL Event::PulseEvent()
{
	return ::PulseEvent(m_hObject);
}
BOOL Event::ResetEvent()
{
	return ::ResetEvent(m_hObject);
}


