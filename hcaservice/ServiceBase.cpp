#include "stdafx.h"
#include "ServiceBase.h"

#include <cassert>

ServiceBase * ServiceBase::m_serviceInstance = NULL;

ServiceBase::ServiceBase(void)
{
}


ServiceBase::~ServiceBase(void)
{
}

ServiceBase::ServiceBase(const CString & name,
	const CString & displayName,
	DWORD dwStartType,
	DWORD dwErrCtrlType,
	DWORD dwAcceptedCmds,
	const CString & depends,
	const CString & account,
	const CString & password)
	: m_name(name)
    , m_displayName(displayName)
    , m_dwStartType(dwStartType)
    , m_dwErrorCtrlType(dwErrCtrlType)
	, m_dwAcceptedCmds(dwAcceptedCmds)
    , m_depends(depends)
    , m_account(account)
	, m_password(password)
	, m_svcStatusHandle(NULL) {

		m_hasPass = !m_depends.IsEmpty();
		m_hasAcc = !m_account.IsEmpty();
		m_hasPass = !m_password.IsEmpty();

		m_svcStatus.dwControlsAccepted = dwAcceptedCmds;
		m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		m_svcStatus.dwCurrentState = SERVICE_START_PENDING;
		m_svcStatus.dwServiceSpecificExitCode = 0;
		m_svcStatus.dwWin32ExitCode = NO_ERROR;
		m_svcStatus.dwCheckPoint = 0;
		m_svcStatus.dwWaitHint = 0;
}


void ServiceBase::SetStatus(DWORD dwState, DWORD dwErrCode /* = NO_ERROR */, DWORD dwWait /* = 0 */) {
	m_svcStatus.dwCurrentState = dwState;
	m_svcStatus.dwWin32ExitCode = dwErrCode;
	m_svcStatus.dwWaitHint = dwWait;
	m_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP 
									| SERVICE_ACCEPT_SHUTDOWN
									| SERVICE_ACCEPT_SESSIONCHANGE;
	::SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

void ServiceBase::WriteEventLog(const CString & msg, WORD type /* = EVENTLOG_INFORMATION_TYPE */) {
	if(msg.IsEmpty()) return;

	HANDLE hSource = RegisterEventSource(NULL, m_name);
	if(hSource) {
		const TCHAR * msgData[2] = {m_name, msg};
		ReportEvent(hSource, type, 0, 0, NULL, 2, 0, msgData, NULL);
		DeregisterEventSource(hSource);
	}
}

void WINAPI ServiceBase::SvcMain(DWORD argc, TCHAR * argv[]) {
	assert(m_serviceInstance);
	m_serviceInstance->m_svcStatusHandle = ::RegisterServiceCtrlHandlerEx(m_serviceInstance->GetName(), ServiceCtrlHandler, NULL);

	if (!m_serviceInstance->m_svcStatusHandle) {
		m_serviceInstance->WriteEventLog(_T("Can't set service control handler."), EVENTLOG_ERROR_TYPE);
		return;
	}

	m_serviceInstance->Start(argc, argv);
}

DWORD WINAPI ServiceBase::ServiceCtrlHandler(DWORD ctrlCode, DWORD evtType, void *evtData, void *context) {
	switch (ctrlCode) {
	case SERVICE_CONTROL_STOP:
		m_serviceInstance->Stop();
		break;

	case SERVICE_CONTROL_PAUSE:
		m_serviceInstance->Suspend();
		break;

	case SERVICE_CONTROL_CONTINUE:
		m_serviceInstance->Resume();
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		m_serviceInstance->Shutdown();
		break;

	case SERVICE_CONTROL_PRESHUTDOWN:
		m_serviceInstance->PerShutDown();
		break;

	case SERVICE_CONTROL_SESSIONCHANGE:
		m_serviceInstance->SessionChange(evtType, reinterpret_cast<WTSSESSION_NOTIFICATION*>(evtData));
		break;

	default:
		break;
	}

	return 0;
}

bool ServiceBase::RunService(ServiceBase * svc) {
	m_serviceInstance = svc;
	TCHAR * svnName = const_cast<CString&> (m_serviceInstance->GetName()).GetBuffer();

	SERVICE_TABLE_ENTRY tableEntry[] = {
		{
			svnName, SvcMain
		},
		{
			NULL, NULL
		}
	};

	return ::StartServiceCtrlDispatcher(tableEntry) == TRUE;
}

void ServiceBase::Start(DWORD argc, TCHAR * argv[]) {
	SetStatus(SERVICE_START_PENDING);
	bool bResult = OnStart(argc, argv);
	if(!bResult) {
		SetStatus(SERVICE_STOPPED);
		return;
	}
	SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Stop() {
	SetStatus(SERVICE_STOP_PENDING);
	OnStop();
	SetStatus(SERVICE_STOPPED);
}

void ServiceBase::Suspend() {
	SetStatus(SERVICE_PAUSE_PENDING);
	OnSuspend();
	SetStatus(SERVICE_PAUSED);
}

void ServiceBase::Resume() {
	SetStatus(SERVICE_CONTINUE_PENDING);
	OnResume();
	SetStatus(SERVICE_RUNNING);
}

void ServiceBase::Shutdown() {
	SetStatus(SERVICE_STOP_PENDING);
	OnShutdown();
	SetStatus(SERVICE_STOPPED);
}

void ServiceBase::PerShutDown() {
	OnPerShutdown();
}

void ServiceBase::SessionChange(DWORD evtType, WTSSESSION_NOTIFICATION * notification) {
	OnSessionChange(evtType, notification);
}

void ServiceBase::NetBindChange(DWORD evtType) {
	OnNetBindChange(evtType);
}