#ifndef _HCASERVICE_NTRIPPROTOCOL_H_
#define _HCASERVICE_NTRIPPROTOCOL_H_

#include "Singleton.h"

class NtripFormatProtocol : public Singleton<NtripFormatProtocol>
{
public:
	NtripFormatProtocol(void);
	~NtripFormatProtocol(void);

public:
	int EncodeAuthRequest(const char * mountpoint, const char * username, const char * password, char * outBuffer, int length);
	int EncodeNmeaRequest(const char * mountpoint, const char * gga, char * outBuffer, int length);

	int DecodeAuthResponse(char * inBuffer, int length);
};

#endif /*_HCASERVICE_NTRIPPROTOCOL_H_*/
