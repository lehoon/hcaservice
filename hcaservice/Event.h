#ifndef _HCASERVICE_EVENT_H
#define _HCASERVICE_EVENT_H

#include <windows.h>

class Event
{
public:
	explicit Event(BOOL bInitiallyOwn = FALSE, BOOL bManualReset = FALSE,
		LPCTSTR lpszNAme = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL);

public:
	BOOL SetEvent();
	BOOL PulseEvent();
	BOOL ResetEvent();

public:
	virtual ~Event();
	operator HANDLE() const { return m_hObject; }
	HANDLE  m_hObject;
};

#endif/*_HCASERVICE_EVENT_H*/
