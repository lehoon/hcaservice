#include "StdAfx.h"

#include <time.h>

#include "ScheduleTaskWorker.h"


CScheduleTaskWorker::CScheduleTaskWorker(void)
{
}


CScheduleTaskWorker::~CScheduleTaskWorker(void)
{
}

bool CScheduleTaskWorker::InitTask() {
	return true;
}

bool CScheduleTaskWorker::ExitTask() {
	return true;
}

bool CScheduleTaskWorker::Work() {
	DeleteLogFile();
	Sleep(5000);
	return true;
}

bool CScheduleTaskWorker::DeleteLogFile() {
	CString modulePath;
	if(::GetModuleFileName(NULL, modulePath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH) == 0) {
		return false;
	}

	int nPos = modulePath.ReverseFind(_T('\\'));
	modulePath = modulePath.Left(nPos + 1);

	TCHAR logFilePatten[256] = {0};
	wsprintf(logFilePatten, _T("%s*.log*"), modulePath);

	BOOL bFinished = TRUE;
	WIN32_FIND_DATA  findFileData;
	ZeroMemory(&findFileData, sizeof(WIN32_FIND_DATA));

	time_t now;
	time(&now);

	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = ::FindFirstFile(logFilePatten, &findFileData);
	if(hFile == INVALID_HANDLE_VALUE) {
		return true;
	}

	do
	{
		if(0 == (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			ULARGE_INTEGER ui;
			ui.HighPart = findFileData.ftLastWriteTime.dwHighDateTime;
			ui.LowPart = findFileData.ftLastWriteTime.dwLowDateTime;
			time_t filecreatetime = (time_t) ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);

			if(now - filecreatetime > 24 * 3600 * 3) {
				::DeleteFile(modulePath + findFileData.cFileName);
			}
		}

		bFinished = ::FindNextFile(hFile, &findFileData);
	} while (bFinished);

	FindClose(hFile);
	return true;
}