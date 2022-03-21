#ifndef _HCASERVICE_UTILS_H_
#define _HCASERVICE_UTILS_H_

#include "stdafx.h"
#include "base_type.h"
#include <Windows.h>
#include <string>
#include <sstream>

class Utils
{
private:
	Utils(void);
	~Utils(void);
public:
	static std::string CString2string(CString m_str);
	static int WcharToChar(char* pDest,CString& pSource,int ilen);
	static void Bcd2Decimal(const unsigned char *bcd, unsigned int length, unsigned char * pOutBuffer, unsigned int outLength);

	static unsigned int GetBitUInt(const unsigned char *buff, int pos, int len);
	static void SetBitUInt(unsigned char *buff, int pos, int len, unsigned int data);

	static unsigned char * DWORD_BToL(unsigned char * buf);
	static unsigned char * WORD_BToL(unsigned char * buf);

	static void String2Hex(char *t, const char *s, int l);

public:
	static Message * GetRawMessage(unsigned short len);
	static void FreeRawMessage(Message * msg);
};

#endif /*_HCASERVICE_UTILS_H_*/