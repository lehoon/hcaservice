#include "stdafx.h"

#include "NemaFormatProtocol.h"

NemaFormatProtocol::NemaFormatProtocol(void)
{
}

NemaFormatProtocol::~NemaFormatProtocol(void)
{
}

/*
param:
       cInBuffer    输入缓冲区
	   nInLen       缓冲区长度
	   nIndex       包头位置
	   nOutLen      包的长度
	   nType        数据类型
*/
int NemaFormatProtocol::ParserDataFrame(const unsigned char * cInBuffer, int nInLen, int &nIndex, int &nOutLen, int &nType) {
	if(cInBuffer == NULL || nInLen < 8) {
		return -1; //长度不够
	}

	//查找包头的位置
	nIndex = GetPackHeaderPos(cInBuffer, nInLen, nType);

	if (-1 == nIndex) {
		return -2;
	}

	//剩余有效长度
	int nVlength = nInLen - nIndex;

	//查找*位置，找到之后+2，因为*后两位是校验位
	int eEnd = GetPackCRCPos(cInBuffer + nIndex, nVlength);
	if (-1 == eEnd) {
		return -3;
	}

	eEnd += 3;
	nOutLen = eEnd;

	if(nVlength < nOutLen) {
		return -4;
	}

	//查看校验位和包尾是否正确
	if (!this->_IsTrueSrc((unsigned char *)cInBuffer + nIndex, nOutLen)) {
		return -5;
	}

	return 0;
}

int NemaFormatProtocol::ParserGPGGA(unsigned char * inputBuffer, int &inLen, PGPGGA out) {
	if (out == NULL) {
		return -1;
	}

	if(inputBuffer == NULL || inLen == 0) {
		return -2;
	}

	if(QueryCommaCount(inputBuffer, inLen) < 14) return -1;

	char *token = NULL, *saveToken = NULL;
	token = strtok((char *) inputBuffer, ",");
	int index = 0;
	while (token != NULL) {
		if(index == 1) {
			out->hour = NumerStringToInt(token, 2);
			out->minute = NumerStringToInt((char *) &token[2], 2);
			out->second = NumerStringToInt((char *) &token[4], 2);
		} else if(index == 6) {
			out->state = atoi(token);
		} else if(index == 7) {
			out->starnum = atoi(token);
		} else if(index == 9) {
			out->alt = atoi(token);
		}

		index++;
		token = strtok(NULL, ",");
	}

	return 0;
}

int NemaFormatProtocol::ParserGPRMC(unsigned char * inputBuffer, int &inLen, PGPRMC out) {
	if (out == NULL) {
		return -1;
	}

	if(inputBuffer == NULL || inLen == 0) {
		return -2;
	}

	if(QueryCommaCount(inputBuffer, inLen) < 12) return -1;

	char *token = NULL, *saveToken = NULL;
	token = strtok((char *) inputBuffer, ",");
	int index = 0;
	while (token != NULL) {
		if(index == 1) {
			out->hour = NumerStringToInt(token, 2);
			out->minute = NumerStringToInt((char *) &token[2], 2);
			out->second = NumerStringToInt((char *) &token[4], 2);
		} else if(index == 2) {
			if(*token == 'A') {
				out->state = 1;
			} else {
				out->state = 0;
			}
		} else if(index == 3) {
			double temp = NumerStringToInt(token, 2) + atof(&token[2]) / 60;
			out->latitude = (int) (temp * 1000000);
		} else if(index == 4) {
			if(*token == 'S') {
				out->latitude = 0 - out->latitude;
				out->latdir = 1;
			} else {
				out->latdir = 0;
			}
		} else if(index == 5) {
			double temp = NumerStringToInt(token, 3) + atof(&token[3]) / 60;
			out->longitude = (int) (temp * 1000000);
		} else if(index == 6) {
			if(*token == 'W') {
				out->londir = 1;
				out->longitude = 0 - out->longitude;
			} else {
				out->londir = 0;
			}
		} else if(index == 7) {
			if(out->state == 1)
				out->speed = (int) (atof(token) * 1852.0);
			else 
				out->speed = 0;

			if(out->speed < 5000) {
				out->speed = 0;
			}
		} else if(index == 8) {
			out->course = (int) atof(token);
		} else if(index == 9) {
			out->day = NumerStringToInt(token, 2);
			out->month = NumerStringToInt((char *) &token[2], 2);
			out->year = NumerStringToInt((char *) &token[4], 2);
		}

		index++;
		token = strtok(NULL, ",");
	}

	return 0;
}

int NemaFormatProtocol::GetPackHeaderPos(const unsigned char *cInBuffer, const int nInLen, int & type) const {
	int nIndex = -1;
	//查找包头  gpgga/gprmc
	for (int i = 0; i < nInLen; i++) {
		if(cInBuffer[i + 3] == 'G' && cInBuffer[i + 4] == 'G' && cInBuffer[i + 5] == 'A') {
			type = 0;
			nIndex = i;
			break;
		} else if(cInBuffer[i + 3] == 'R' && cInBuffer[i + 4] == 'M' && cInBuffer[i + 5] == 'C'){
			type = 1;
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

int NemaFormatProtocol::GetPackCRCPos(const unsigned char *cInBuffer, const int nInLen) const {
	for (int i = 0; i < nInLen; ++i) {
		if (cInBuffer[i] == '*')
			return i;
	}

	return -1;
}

bool NemaFormatProtocol::_IsTrueSrc(unsigned char *cInBuffer, int &nOutLen) {
	short sCrc = 0;
	char ch[4] = {0};
	memcpy(ch, cInBuffer + nOutLen - 2, 2);
	sCrc = (short)strtoul(ch, nullptr, 16);
	return CRC_Check(cInBuffer, nOutLen, sCrc);
}

bool NemaFormatProtocol::CRC_Check(unsigned char *cInBuffer, int nOutLen, short sSRC) {
	int checksum = 0;
	for(int i=1;i<nOutLen-3;i++)
		checksum ^= cInBuffer[i];

	return (sSRC == checksum) ? true:false;
}

int NemaFormatProtocol::QueryCommaCount(unsigned char *cInBuffer, const int &nInLen) {
	if (cInBuffer == NULL || nInLen == 0) {
		return 0;
	}

	int count = 0;
	for (int i = 0; i < nInLen; i++) {
		if(cInBuffer[i] == ',') {
			count++;
		}
	}

	return count;
}

int NemaFormatProtocol::NumerStringToInt(char * str, int len) {
	int num = 0; 

	for (int i = 0; i < len; i++, str++) {
		num = num * 10 + (*str - '0');
	}

	return num;
}
