#include "stdafx.h"
#include "ServiceInstall.h"

namespace {
	class ServiceHandle {
	public:
		ServiceHandle(SC_HANDLE handle) : m_handle(handle) {
		}

		~ServiceHandle() {
			if (m_handle) {
				::CloseServiceHandle(m_handle);
			}
		}

		operator SC_HANDLE() {
			return m_handle;
		}

	private:
		SC_HANDLE m_handle;
	};
}


ServiceInstall::ServiceInstall(void)
{
}

bool ServiceInstall::Install(const ServiceBase & service) {
	CString escapePath;
	TCHAR * modulePath = escapePath.GetBufferSetLength(MAX_PATH);

	if(::GetModuleFileName(NULL, modulePath, MAX_PATH) == 0) {
		_tprintf(_T("Couldn't get module file name:%d\n"), ::GetLastError());
		return false;
	}

	escapePath.ReleaseBuffer();
	escapePath.Remove(_T('\"'));
	escapePath = _T('\"') + escapePath + _T(" service \"");

	ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);

	if (!svcControlManager) {
		_tprintf(_T("Couldn't open service control manager: %d\n"), ::GetLastError());
		return false;
	}

	const CString & depends = service.GetDependcies();
	const CString & account = service.GetAccount();
	const CString & password = service.GetPassword();

	ServiceHandle servHandle = ::CreateService(svcControlManager,
		service.GetName(),
		service.GetDisplayName(),
		SERVICE_QUERY_STATUS,
		SERVICE_WIN32_OWN_PROCESS,
		service.GetStartType(),
		service.GetErrorControlType(),
		escapePath,
		NULL,
		NULL,
		(depends.IsEmpty() ? NULL : depends.GetString()),
		(account.IsEmpty() ? NULL : account.GetString()),
		(password.IsEmpty() ? NULL : password.GetString()));

	if (!servHandle) {
		_tprintf(_T("Couldn't create service : %d\n"), ::GetLastError());
		return false;
	}

	ServiceInstall::AddServiceDescription(service);
	ServiceInstall::DelayedStartService(service, TRUE);
	//ServiceInstall::SetServiceRestartOnStop(service);
	return true;
}

bool ServiceInstall::UnInstall(const ServiceBase & service) {
	ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!svcControlManager) {
		_tprintf(_T("Couldn't open service control manager: %d\n"), ::GetLastError());
		return false;
	}

	ServiceHandle svcHandle = ::OpenService(svcControlManager, service.GetName(),
		SERVICE_QUERY_STATUS | SERVICE_STOP | DELETE);
	if (!svcHandle) {
		_tprintf(_T("Couldn't open service : %d\n"), ::GetLastError());
		::CloseServiceHandle(svcControlManager);
		return false;
	}

	SERVICE_STATUS svcStatus = {};
	if(::ControlService(svcHandle, SERVICE_CONTROL_STOP, &svcStatus)) {
		_tprintf(_T("Stop service %s"), service.GetName());

		while (::QueryServiceStatus(svcHandle, &svcStatus)) {
			if (svcStatus.dwCurrentState != SERVICE_STOP_PENDING) {
				break;
			}
		}

		if (svcStatus.dwCurrentState != SERVICE_STOPPED) {
			_tprintf(_T("Stop service Failed."));
		} else {
			_tprintf(_T("Stop service Success."));
		}
	}

	if ( !::DeleteService(svcHandle) ) {
		_tprintf(_T("Delete service Faild [%d]"), ::GetLastError());
		return false;
	}

	if (svcControlManager) {
		::CloseServiceHandle(svcControlManager);
		svcControlManager = NULL;
	}

	if (svcHandle) {
		::CloseServiceHandle(svcHandle);
		svcHandle = NULL;
	}

	return true;
}

void ServiceInstall::AddServiceDescription(const ServiceBase & service) {
	ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!svcControlManager) {
		_tprintf(_T("Couldn't open service control manager: %d\n"), ::GetLastError());
		return;
	}

	SC_LOCK scLock = ::LockServiceDatabase(svcControlManager);
	if(scLock != NULL) {
		ServiceHandle shService = ::OpenService(svcControlManager, service.GetName(), SERVICE_CHANGE_CONFIG);
		if(shService != NULL) {
			SERVICE_DESCRIPTION description;
			description.lpDescription = _T("移动定位上位机软件,通过交通部jt808协议上传位置.");
			BOOL retcode = ChangeServiceConfig2(shService, SERVICE_CONFIG_DESCRIPTION, &description);
			DWORD errcode = GetLastError();
			::CloseServiceHandle(shService);
		}
		::UnlockServiceDatabase(scLock);
	}
	DWORD errcode = GetLastError();
	::CloseServiceHandle(svcControlManager);
}

void ServiceInstall::DelayedStartService(const ServiceBase & service, BOOL autoStart) {
	ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!svcControlManager) {
		_tprintf(_T("Couldn't open service control manager: %d\n"), ::GetLastError());
		return;
	}

	SC_LOCK scLock = ::LockServiceDatabase(svcControlManager);
	if(scLock != NULL) {
		ServiceHandle shService = ::OpenService(svcControlManager, service.GetName(), SERVICE_CHANGE_CONFIG);
		if(shService != NULL) {
			SERVICE_DELAYED_AUTO_START_INFO info;
			info.fDelayedAutostart = autoStart;
			ChangeServiceConfig2(shService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &info);
			::CloseServiceHandle(shService);
		}
		::UnlockServiceDatabase(scLock);
	}
	DWORD errcode = GetLastError();
	::CloseServiceHandle(svcControlManager);
}

void ServiceInstall::SetServiceRestartOnStop(const ServiceBase & service) {
	ServiceHandle svcControlManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!svcControlManager) {
		_tprintf(_T("Couldn't open service control manager: %d\n"), ::GetLastError());
		return;
	}

	SC_LOCK scLock = ::LockServiceDatabase(svcControlManager);
	if(scLock != NULL) {
		ServiceHandle shService = ::OpenService(svcControlManager, service.GetName(), SERVICE_CHANGE_CONFIG);
		if(shService != NULL) {
			SERVICE_FAILURE_ACTIONS sfBuff = {0};
			sfBuff.lpRebootMsg = NULL;
			sfBuff.dwResetPeriod = 3600 * 24;
			SC_ACTION action[3];
			action[0].Delay = 60 * 1000;
			action[0].Type = SC_ACTION_RESTART;

			action[1].Delay = 0;
			action[1].Type = SC_ACTION_RESTART;

			action[2].Delay = 0;
			action[2].Type = SC_ACTION_RESTART;

			sfBuff.cActions = 3;
			sfBuff.lpsaActions = action;
			sfBuff.lpCommand = NULL;

			ChangeServiceConfig2(shService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfBuff);
			::CloseServiceHandle(shService);
		}
		::UnlockServiceDatabase(scLock);
	}
	DWORD errcode = GetLastError();
	::CloseServiceHandle(svcControlManager);
}
