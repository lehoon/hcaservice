// hcaservice.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CrashDumper.h"
#include "DispatchService.h"

#pragma data_seg(".share")
long share_count = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.share,RWS")

int _tmain(int argc, _TCHAR* argv[])
{
	if(share_count > 0) {
		return 0;
	}

	share_count++;
	SetupMiniDumpHandle();
	TcpTransferBase::InitSocketEnv();
	DispatchService service(_T("hcaservice"), _T(" 移动定位上位机系统"), SERVICE_AUTO_START);

	if (argc > 1) {
		if (_tcscmp(argv[1], _T("install")) == 0) {
			ServiceInstall::Install(service);
		} else if (_tcscmp(argv[1], _T("uninstall")) == 0) {
			ServiceInstall::UnInstall(service);
		} else if (_tcscmp(argv[1], _T("console")) == 0)  {
			service.Run(_T("console"));
		} else if (_tcscmp(argv[1], _T("start")) == 0)  {
			service.Run(_T("start"));
		} else if (_tcscmp(argv[1], _T("stop")) == 0)  {
			service.Run(_T("stop"));
		} else {
			service.Run();
		}
	} else {
		service.Run();
	}

	return 0;
}


