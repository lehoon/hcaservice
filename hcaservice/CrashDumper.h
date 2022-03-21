#ifndef _HCASERVICE_CRASHDUMPER_H
#define _HCASERVICE_CRASHDUMPER_H

#include <windows.h>
#include <DbgHelp.h>
#include <stdlib.h>

#pragma comment(lib, "dbghelp.lib")

inline BOOL IsDataSectionNeeded(const WCHAR * pModuleName) {
	if(pModuleName == NULL) {
		return FALSE;
	}

	TCHAR szFileName[_MAX_FNAME] = L"";
	TCHAR szDrive[_MAX_DRIVE] = L"";
	TCHAR szDir[_MAX_DIR] = L"";
	TCHAR szExt[_MAX_EXT] = L"";

#if (_MSC_VER > 1300)
	_wsplitpath_s(pModuleName, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szFileName, _MAX_FNAME, szExt, _MAX_EXT);
#else
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);
#endif
	
	if(_wcsicmp(szFileName, L"ntdll") == 0) return TRUE;
	return FALSE;
}

inline BOOL CALLBACK MiniDumpCallBack(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) {
	if(pInput == NULL || pOutput == NULL) {
		return FALSE;
	}

	switch(pInput->CallbackType) {
	case ModuleCallback:
		if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg) 
			if(!IsDataSectionNeeded(pInput->Module.FullPath))
				pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
	case IncludeModuleCallback:
	case IncludeThreadCallback:
	case ThreadExCallback:
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

inline void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCTSTR strFileName) {
	HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile != NULL && (hFile != INVALID_HANDLE_VALUE)) {
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = NULL;

		MINIDUMP_CALLBACK_INFORMATION mdci;
		mdci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE) MiniDumpCallBack;
		mdci.CallbackParam = 0;


		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mdci);
		::CloseHandle(hFile);
	}
}

LONG __stdcall LeUnhandledExceptionFilter( PEXCEPTION_POINTERS pExceptionInfo ) {
	CreateMiniDump(pExceptionInfo, _T("core.dump"));
	return EXCEPTION_EXECUTE_HANDLER;
}

void DisableSetUnHandleExceptionFilter() {
	void * addr = (void *) GetProcAddress(LoadLibrary(_T("kernel32.dll")), "SetUnhandledExceptionFilter");

	if(addr) {
		unsigned char code[16] = {0};
		int size = 0;
		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		::VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
		::WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
		::VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
	}
}

inline void SetupMiniDumpHandle() {
	::SetUnhandledExceptionFilter(LeUnhandledExceptionFilter);
	DisableSetUnHandleExceptionFilter();
}


#endif /*_HCASERVICE_CRASHDUMPER_H*/


