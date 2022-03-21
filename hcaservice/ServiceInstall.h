#ifndef _HCASERVICE_SRTVICE_INSTALL_H_
#define _HCASERVICE_SRTVICE_INSTALL_H_

#include "ServiceBase.h"

class ServiceInstall
{
public:
	static bool Install(const ServiceBase & service);
	static bool UnInstall(const ServiceBase & service);
	static void SetServiceRestartOnStop(const ServiceBase & service);
	static void AddServiceDescription(const ServiceBase & service);
	static void DelayedStartService(const ServiceBase & service, BOOL autoStart);
private:
	ServiceInstall(void);

	DISABLE_COPY_AND_MOVE(ServiceInstall)
};

#endif /*_HCASERVICE_SRTVICE_INSTALL_H_*/