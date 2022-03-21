#ifndef _HCASERVICE_NEMAPROTOCOL_H_
#define _HCASERVICE_NEMAPROTOCOL_H_

#include "base_type.h"

#include "Singleton.h"
class NemaFormatProtocol : public Singleton<NemaFormatProtocol>
{
public:
	NemaFormatProtocol(void);
	~NemaFormatProtocol(void);

public:
	int ParserDataFrame(const unsigned char * cInBuffer, int nInLen, int &nIndex, int &nOutLen, int &nType);
	int ParserGPGGA(unsigned char * inputBuffer, int &inLen, PGPGGA out);
	int ParserGPRMC(unsigned char * inputBuffer, int &inLen, PGPRMC out);

private:
	inline int GetPackHeaderPos(const unsigned char *cInBuffer, const int nInLen, int & type) const;
	int GetPackCRCPos(const unsigned char *cInBuffer, const int nInLen) const;
	bool _IsTrueSrc(unsigned char *cInBuffer, int &nOutLen);
	bool CRC_Check(unsigned char *cInBuffer, int nOutLen, short sSRC);
	int QueryCommaCount(unsigned char *cInBuffer, const int &nInLen);
	int NumerStringToInt(char * str, int len);
};

#endif /*_HCASERVICE_NEMAPROTOCOL_H_*/
