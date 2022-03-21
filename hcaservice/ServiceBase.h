#ifndef _HCASERVICE_SERVICE_BASE_H_
#define _HCASERVICE_SERVICE_BASE_H_

#include "stdafx.h"

#include <atlstr.h>
#include "copy_disable.h"

class ServiceBase
{
public:
	ServiceBase(void);
	virtual ~ServiceBase(void);

	bool Run() {
		return RunService(this);
	}

	const CString & GetName() const { return m_name; }
	const CString & GetDisplayName() const { return m_displayName; }
	const CString & GetDependcies() const { return m_depends; }

	const CString & GetAccount() const { return m_account; }
	const CString & GetPassword() const { return m_password; }

	const DWORD GetStartType() const { return m_dwStartType; }
	const DWORD GetErrorControlType() const { return m_dwErrorCtrlType; }

protected:
	ServiceBase(const CString & name,
		const CString & displayName,
		DWORD dwStartType,
		DWORD dwErrCtrlType = SERVICE_ERROR_NORMAL,
		DWORD dwAcceptedCmds = SERVICE_ACCEPT_STOP 
							| SERVICE_ACCEPT_SHUTDOWN 
							| SERVICE_ACCEPT_PRESHUTDOWN 
							| SERVICE_ACCEPT_NETBINDCHANGE,
		const CString & depends = _T(""),
		const CString & account = _T(""),
		const CString & password = _T(""));

	void SetStatus(DWORD dwState, DWORD dwErrCode = NO_ERROR, DWORD dwWait = 0);
	void WriteEventLog(const CString & msg, WORD type = EVENTLOG_INFORMATION_TYPE);

	virtual bool OnStart(DWORD argc, TCHAR * argv[]) = 0;
	virtual bool OnStop() {return true;}
	virtual bool OnSuspend() {return true;}
	virtual bool OnResume() {return true;}
	virtual bool OnShutdown() {return true;}
	virtual bool OnPerShutdown() {return true;}
	virtual bool OnNetBindChange(DWORD evtType) { return true; }
	virtual bool OnSessionChange(DWORD evtType, WTSSESSION_NOTIFICATION * notification) {return true;}
private:
	static void WINAPI SvcMain(DWORD argc, TCHAR * argv[]);
	static DWORD WINAPI ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType, void *evtData, void *context);
	
	static bool RunService(ServiceBase * svc);

	void Start(DWORD argc, TCHAR * argv[]);
	void Stop();
	void Suspend();
	void Resume();
	void Shutdown();
	void PerShutDown();
	void NetBindChange(DWORD evtType);
	void SessionChange(DWORD evtType, WTSSESSION_NOTIFICATION * notification);

	CString m_name;
	CString m_displayName;
	CString m_depends;
	CString m_account;
	CString m_password;

	DWORD m_dwStartType;
	DWORD m_dwErrorCtrlType;
	DWORD m_dwAcceptedCmds;

	bool m_hasDepends;
	bool m_hasAcc;
	bool m_hasPass;

	SERVICE_STATUS m_svcStatus;
	SERVICE_STATUS_HANDLE m_svcStatusHandle;

	static ServiceBase * m_serviceInstance;
	DISABLE_COPY_AND_MOVE(ServiceBase)
};

#endif /*_HCASERVICE_SERVICE_BASE_H_*/


