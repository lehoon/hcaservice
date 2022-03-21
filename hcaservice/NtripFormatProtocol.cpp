#include "StdAfx.h"
#include "Utils.h"
#include "NtripFormatProtocol.h"

static char * encode_base64(const char *buf, const long size, char *base64Char);

NtripFormatProtocol::NtripFormatProtocol(void)
{
}


NtripFormatProtocol::~NtripFormatProtocol(void)
{
}

int NtripFormatProtocol::EncodeAuthRequest(const char * mountpoint, const char * username, const char * password, char * outBuffer, int length) {
	int nIndex = 0, msgLength = 0;
	msgLength = sprintf(outBuffer, "GET /%s HTTP/1.0\r\n", mountpoint);
	nIndex += msgLength;

	char * agent = "User-Agent: NTRIP product\r\n";
	memcpy(outBuffer + nIndex, agent, strlen(agent));
	nIndex += strlen(agent);

	char authcode[128] = {0};
	sprintf(authcode, "%s:%s", username, password);
	char base64[128] = {0};
	encode_base64(authcode, strlen(authcode), base64);

	msgLength = sprintf(outBuffer + nIndex, "Authorization: Basic %s\r\n\r\n", base64);
	nIndex += msgLength;
	return nIndex;
}

int NtripFormatProtocol::EncodeNmeaRequest(const char * mountpoint, const char * gga, char * outBuffer, int length) {
	int nIndex = 0, msgLength = 0;
	msgLength = sprintf(outBuffer, "GET /%s HTTP/1.0\r\n", mountpoint);
	nIndex += msgLength;

	char * accept = "Accept: rtk/rtcm, dgps/rtcm\r\n";
	memcpy(outBuffer + nIndex, accept, strlen(accept));
	nIndex += strlen(accept);

	char * agent = "User-Agent: NTRIP Survey-Controller-15.0\r\n";
	memcpy(outBuffer + nIndex, agent, strlen(agent));
	nIndex += strlen(agent);

	msgLength = sprintf_s(outBuffer + nIndex, length - nIndex, "%s\r\n", gga);
	nIndex += msgLength;
	return nIndex;
}

int NtripFormatProtocol::DecodeAuthResponse(char * inBuffer, int length) {
	if(inBuffer == NULL || length == 0) return -1;

	for(int i = 0; i < length; i++) {
		if(0 == strncmp((const char *) inBuffer + i, "ICY 200 OK", strlen("ICY 200 OK"))) {
			return 0;
		}
	}

	return -1;
}

static const char *ALPHA_BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char * encode_base64(const char *buf, const long size, char *base64Char) {
	int a = 0;
	int i = 0;
	while (i < size) {
		char b0 = buf[i++];
		char b1 = (i < size) ? buf[i++] : 0;
		char b2 = (i < size) ? buf[i++] : 0;

		int int63 = 0x3F; //  00111111
		int int255 = 0xFF; // 11111111
		base64Char[a++] = ALPHA_BASE[(b0 >> 2) & int63];
		base64Char[a++] = ALPHA_BASE[((b0 << 4) | ((b1 & int255) >> 4)) & int63];
		base64Char[a++] = ALPHA_BASE[((b1 << 2) | ((b2 & int255) >> 6)) & int63];
		base64Char[a++] = ALPHA_BASE[b2 & int63];
	}
	switch (size % 3) {
	case 1:
		base64Char[--a] = '=';
	case 2:
		base64Char[--a] = '=';
	}
	return base64Char;
}

static char * decode_base64(const char *base64Char, const long base64CharSize, char *originChar, long originCharSize) {
	int toInt[128] = {-1};
	for (int i = 0; i < 64; i++) {
		toInt[ALPHA_BASE[i]] = i;
	}
	int int255 = 0xFF;
	int index = 0;
	for (int i = 0; i < base64CharSize; i += 4) {
		int c0 = toInt[base64Char[i]];
		int c1 = toInt[base64Char[i + 1]];
		originChar[index++] = (((c0 << 2) | (c1 >> 4)) & int255);
		if (index >= originCharSize) {
			return originChar;
		}
		int c2 = toInt[base64Char[i + 2]];
		originChar[index++] = (((c1 << 4) | (c2 >> 2)) & int255);
		if (index >= originCharSize) {
			return originChar;
		}
		int c3 = toInt[base64Char[i + 3]];
		originChar[index++] = (((c2 << 6) | c3) & int255);
	}
	return originChar;
}