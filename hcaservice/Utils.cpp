#include "stdafx.h"

#include "Utils.h"
#include <math.h>


Utils::Utils(void)
{
}


Utils::~Utils(void)
{
}


std::string Utils::CString2string(CString m_str) {
	char ch[MAX_PATH + 1] = {0};
	WcharToChar(ch, m_str, m_str.GetLength()*2);
	return ch;
}

int Utils::WcharToChar(char* pDest,CString& pSource,int ilen) {
	ilen -= 1;
	wchar_t * wctext = pSource.GetBuffer(pSource.GetLength()+1);
	int dwMinSize = ::WideCharToMultiByte (CP_ACP, 0, wctext, -1, NULL, 0, NULL, NULL);
	if (dwMinSize > ilen)
	{
		::WideCharToMultiByte(CP_ACP, 0, wctext, -1, pDest, ilen, NULL, NULL);
	}
	else
		::WideCharToMultiByte(CP_ACP, 0, wctext, -1, pDest, dwMinSize, NULL, NULL);
	pDest[dwMinSize + 1] = '\0';
	return dwMinSize - 1;
}

void Utils::Bcd2Decimal(const unsigned char *bcd, unsigned int length, unsigned char * pOutBuffer, unsigned int outLength) {
	int tmp, temp;
	unsigned long long decimal = 0;

	if(length * 2 > outLength) {
		length = (unsigned int) ceil((float) outLength / 2);
	}

	for(unsigned int i = 0; i < length; i++) {
		tmp = ((bcd[i] >> 4) & 0x0F);
		temp = bcd[i] & 0x0F;
		sprintf_s((char *)(pOutBuffer + i * 2), outLength, "%d%d", tmp, temp);
	}
}

unsigned int Utils::GetBitUInt(const unsigned char *buff, int pos, int len) {
	unsigned int bits=0;
	int i;
	for (i=pos;i<pos+len;i++) bits=(bits<<1)+((buff[i/8]>>(7-i%8))&1u);
	return bits;
}

void Utils::SetBitUInt(unsigned char *buff, int pos, int len, unsigned int data) {
	unsigned int mask=1u<<(len-1);
	int i;
	if (len<=0||32<len) return;
	for (i=pos;i<pos+len;i++,mask>>=1) {
		if (data&mask) buff[i/8]|=1u<<(7-i%8); else buff[i/8]&=~(1u<<(7-i%8));
	}
}

unsigned char * Utils::DWORD_BToL(unsigned char * buf) {
	char ch = buf[3];
	buf[3] = buf[0];
	buf[0] = ch;

	ch = buf[2];
	buf[2] = buf[1];
	buf[1] = ch;
	return buf;
}

unsigned char * Utils::WORD_BToL(unsigned char * buf) {
	char ch = buf[1];
	buf[1] = buf[0];
	buf[0] = ch;
	return buf;
}

void Utils::String2Hex(char *t, const char *s, int l) {
	int i;
	unsigned char c;

	if( t == NULL || s == NULL ) return;

	for( i = 0; i < l; i++ )
	{
		c = (s[i] >> 4) & 0x0F;
		t[2*i] = (c <= 0x9) ? c + 0x30 : c + 0x37;

		c = s[i] & 0x0F;
		t[2*i + 1] = (c <= 0x9) ? c + 0x30 : c + 0x37;
	}
	t[2*i] = 0;

	return;
}


Message * Utils::GetRawMessage(unsigned short len) {
	Message* m = new Message;
	m->len = len;
	m->use_len = 0;
	m->ref = 1;
	m->timeout = 0;
	m->seqno = 0;
	m->data = (unsigned char *) new char[len];
	return m;
}

void Utils::FreeRawMessage(Message * msg) {
	if(msg == NULL) return;
	msg->ref--;
	if(msg->ref > 0) return;
	if(msg->data != NULL) {
		delete [] msg->data;
		msg->data = NULL;
	}
	delete msg;
}
