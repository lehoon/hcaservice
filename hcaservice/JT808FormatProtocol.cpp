#include "stdafx.h"

#include "JT808FormatProtocol.h"
#include "Configure.h"
#include "Utils.h"
#include "SequeNoUtil.h"


JT808FormatProtocol::JT808FormatProtocol(void)
{
}


JT808FormatProtocol::~JT808FormatProtocol(void)
{
}

int JT808FormatProtocol::HandlePackageFrame(unsigned char * buf, int len, int & nIndex, int & nLength) {
	nIndex = 0;
	while (nIndex < len) {
		if(buf[nIndex] != 0x7e) {
			int nEnd = nIndex;
			while ((buf[nEnd] != 0x7e) && nEnd < len) nEnd++;

			if(nEnd < len) {
				nIndex = nEnd;
			} else {
				return kError;
			}
		}

		break;
	}

	//ÕÒµ½header
	if(nIndex + 3 > len) return kNeedData;
	const unsigned char * end_buff = FindEx(buf + nIndex, len - nIndex);
	if(end_buff == NULL) return kNeedData;
	nLength = end_buff - (buf + nIndex) + 1;
	return kSuccess;
}

int JT808FormatProtocol::DecodeMessageHeader(unsigned char * buff, int len, MsgHeader *header) {
	unsigned short type = 0;
	unsigned short tmp = 0;
	memcpy(&type, buff + 1, 1);
	memcpy(&tmp, buff + 2, 1);
	header->msgid = (type << 8) | tmp;
	header->length = Utils::GetBitUInt(buff + 3, 6, 10);
	header->encrypt = Utils::GetBitUInt(buff + 3, 3, 3);
	header->package = Utils::GetBitUInt(buff + 3, 0, 3);
	memcpy(header->bcd, buff + 5, 6);
	memcpy(&type, buff + 11, 1);
	memcpy(&tmp, buff + 12, 1);
	header->seqno = (type << 8) | tmp;
	return kSuccess;
}

int JT808FormatProtocol::DecodeRegisterResponse(unsigned char * buf, int len, MsgRegisterResponse & msg) {
	int index = 1 + 12;
	char high, low;
	high = buf[index];
	low = buf[index + 1];
	msg.reqno = high << 8 | low;
	index += 2;
	msg.result = buf[index];
	index += 1;

	if(msg.result > 0) {
		return kSuccess;
	}

	//authcode
	msg.authcode.assign((const char *) (buf + index), msg.header->length - 3);
	return kSuccess;
}

int JT808FormatProtocol::DecodePlateformResponse(unsigned  char * buf, int len, MsgPlateResponse & msg) {
	int index = 1 + 12;
	char high, low;
	high = buf[index];
	low = buf[index + 1];
	index+=2;
	msg.reqno = high << 8 | low;

	high = buf[index];
	low = buf[index + 1];
	index+=2;
	msg.msgid = high << 8 | low;
	msg.result = buf[index];
	return kSuccess;
}

int JT808FormatProtocol::Encode0200Request(Position * position, unsigned char * outBuffer, int &nOutLen, unsigned short seqno) {
	unsigned char buffer[1024] = {0};

	int nBodyIndex = 13;
	//alarm
	nBodyIndex += 4;
	//state
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 0, 1, 1);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 1, 1, position->state); //position state
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 2, 1, position->latdir);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 3, 1, position->londir);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 18, 1, 1);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 19, 1, 1);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 20, 1, 0);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 21, 1, 0);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 22, 1, position->diff == 2 ? 1 : 0);

	//state length
	nBodyIndex += 4;
	//latitude
	memcpy(buffer + nBodyIndex, &position->latitude, 4);
	SWAP_DWORD(buffer + nBodyIndex);
	nBodyIndex += 4;

	memcpy(buffer + nBodyIndex, &position->longitude, 4);
	SWAP_DWORD(buffer + nBodyIndex);
	nBodyIndex += 4;

	memcpy(buffer + nBodyIndex, &position->alt, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	memcpy(buffer + nBodyIndex, &position->speed, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	memcpy(buffer + nBodyIndex, &position->course, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	char time[16] = {0};
	sprintf_s(time, 15, "%02d%02d%02d%02d%02d%02d", position->year, position->month, position->day, position->hour, position->minute, position->second);
	STRING_2_BCD(time, (unsigned char *) buffer + nBodyIndex, 6);
	nBodyIndex += 6;

	//extra infomation
	buffer[nBodyIndex] = 0x31;
	nBodyIndex++;
	buffer[nBodyIndex] = 0x01;
	nBodyIndex++;
	buffer[nBodyIndex] = position->starnum & 0xf;
	nBodyIndex++;

	GeneralHeaderRequest(buffer, nOutLen, nBodyIndex - 13, 0x0200, seqno);
	buffer[nBodyIndex] = (char) Crc8(buffer, nBodyIndex);
	nBodyIndex += 1;
	buffer[nBodyIndex] = 0x7e;
	nBodyIndex += 1;
	return OriginalToTransfer(buffer, nBodyIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::Encode0704Request(Position * position, unsigned char * outBuffer, int &nOutLen, unsigned short seqno) {
	unsigned char buffer[1024] = {0};

	int nBodyIndex = 13;
	//alarm
	nBodyIndex += 4;
	//state
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 1, 1, position->state); //position state
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 2, 1, position->latdir);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 3, 1, position->londir);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 18, 1, 1);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 19, 1, 1);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 20, 1, 0);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 21, 1, 0);
	Utils::SetBitUInt(buffer + nBodyIndex, 31 - 22, 1, position->diff);

	//state length
	nBodyIndex += 4;
	//latitude
	memcpy(buffer + nBodyIndex, &position->latitude, 4);
	SWAP_DWORD(buffer + nBodyIndex);
	nBodyIndex += 4;

	memcpy(buffer + nBodyIndex, &position->longitude, 4);
	SWAP_DWORD(buffer + nBodyIndex);
	nBodyIndex += 4;

	memcpy(buffer + nBodyIndex, &position->alt, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	memcpy(buffer + nBodyIndex, &position->speed, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	memcpy(buffer + nBodyIndex, &position->course, 2);
	SWAP_WORD(buffer + nBodyIndex);
	nBodyIndex += 2;

	char time[16] = {0};
	sprintf_s(time, 15, "%02d%02d%02d%02d%02d%02d", position->year, position->month, position->day, position->hour, position->minute, position->second);
	STRING_2_BCD(time, (unsigned char *) buffer + nBodyIndex, 6);
	nBodyIndex += 6;

	//extra infomation
	buffer[nBodyIndex] = 0x31;
	nBodyIndex++;
	buffer[nBodyIndex] = 0x01;
	nBodyIndex++;
	buffer[nBodyIndex] = position->starnum;
	nBodyIndex++;

	GeneralHeaderRequest(buffer, nOutLen, nBodyIndex - 13, 0x0704, seqno);
	buffer[nBodyIndex] = (char) Crc8(buffer, nBodyIndex);
	nBodyIndex += 1;
	buffer[nBodyIndex] = 0x7e;
	nBodyIndex += 1;
	return OriginalToTransfer(buffer, nBodyIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::EncodeRegisterRequest(unsigned char * outBuffer, int &nOutLen, unsigned short seqno) {
	unsigned char buffer[128] = {0};
	int bodyLength = 37 + Configure::GetInstance().NumberPlate().length();
	int nIndex = GeneralHeaderRequest(buffer, nOutLen, bodyLength, 0x0100, seqno);

	unsigned int province = Configure::GetInstance().Province();
	unsigned int city = Configure::GetInstance().City();
	std::string  manufacturer = Configure::GetInstance().ManuFacturer();
	std::string  model = Configure::GetInstance().Model();
	std::string  unitid = Configure::GetInstance().UnitNo();
	unsigned char color = Configure::GetInstance().Color();
	std::string numberplate = Configure::GetInstance().NumberPlate();

	memcpy(buffer + nIndex, (char *)&province, 2);
	SWAP_WORD(buffer + nIndex);
	nIndex += 2;

	memcpy(buffer + nIndex, (char *)&city, 2);
	SWAP_WORD(buffer + nIndex);
	nIndex += 2;

	memcpy(buffer + nIndex, manufacturer.c_str(), manufacturer.length() > 5 ? 5 : manufacturer.length());
	nIndex += 5;

	memcpy(buffer + nIndex, model.c_str(), model.length() > 20 ? 20 : model.length());
	nIndex += 20;

	memcpy(buffer + nIndex, unitid.c_str(), unitid.length() > 7 ? 7 : unitid.length());
	nIndex += 7;

	buffer[nIndex] = color;
	nIndex += 1;

	memcpy(buffer + nIndex, numberplate.c_str(), numberplate.length());
	nIndex += numberplate.length();

	buffer[nIndex] = (char) Crc8(buffer, nIndex);
	nIndex += 1;
	buffer[nIndex] = 0x7e;
	nIndex += 1;
	return OriginalToTransfer(buffer, nIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::EncodeAuthorRequest(unsigned char * outBuffer, int &nOutLen, std::string &authcode, unsigned short seqno) {
	unsigned char buffer[64] = {0};
	int nIndex = GeneralHeaderRequest(buffer, nOutLen, authcode.length(), 0x0102, seqno);

	memcpy(buffer + nIndex, (char *)authcode.c_str(), authcode.length());
	nIndex += authcode.length();

	buffer[nIndex] = (char) Crc8(buffer, nIndex);
	nIndex += 1;
	buffer[nIndex] = 0x7e;
	nIndex += 1;
	return OriginalToTransfer(buffer, nIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::Encode0x0001Response(unsigned char * outBuffer, int &nOutLen, unsigned short seqno, unsigned short opcode, unsigned char result) {
	std::string phone = Configure::GetInstance().Phone();
	if(phone.size() <= 0) {
		nOutLen = 0;
		return 0;
	}

	unsigned char buffer[32] = {0};
	int nIndex = GeneralHeaderRequest(buffer, nOutLen, 0, 0x0001, seqno);

	memcpy(buffer + nIndex, (char *)&seqno, 2);
	SWAP_WORD(outBuffer + nIndex);
	nIndex += 2;

	memcpy(buffer + nIndex, (char *)&opcode, 2);
	SWAP_WORD(outBuffer + nIndex);
	nIndex += 2;

	buffer[nIndex] = result;
	nIndex += 1;

	buffer[nIndex] = (char) Crc8(buffer, nIndex);
	nIndex += 1;
	buffer[nIndex] = 0x7e;
	nIndex += 1;
	return OriginalToTransfer(buffer, nIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::Encode0x8104Response(unsigned char * outBuffer, int &nOutLen, unsigned short seqno) {
	unsigned char buffer[128] = {0};
	int nIndex = GeneralHeaderRequest(buffer, nOutLen, 0, 0x0104, seqno);

	memcpy(buffer + nIndex, (char *)&seqno, 2);
	SWAP_WORD(outBuffer + nIndex);
	nIndex += 2;

	buffer[nIndex] = (char) Crc8(buffer, nIndex);
	nIndex += 1;
	buffer[nIndex] = 0x7e;
	nIndex += 1;
	return OriginalToTransfer(buffer, nIndex, outBuffer, nOutLen);
}

int JT808FormatProtocol::EncodeHeartBeatRequest(unsigned char * outBuffer, int &nOutLen, unsigned short seqno) {
	std::string phone = Configure::GetInstance().Phone();
	if(phone.size() <= 0) {
		nOutLen = 0;
		return 0;
	}

	unsigned char buffer[16] = {0};
	int nIndex = GeneralHeaderRequest(buffer, nOutLen, 0, 0x0002, seqno);
	buffer[nIndex] = (char) Crc8(buffer, nIndex);
	buffer[nIndex + 1] = 0x7e;
	return OriginalToTransfer(buffer, 15, outBuffer, nOutLen);
}

//Ìî³ä13¸ö×Ö½Ú
int JT808FormatProtocol::GeneralHeaderRequest(unsigned char * outBuffer, int &nInLen, int msgLen, unsigned short opcode, unsigned short seqno) {
	outBuffer[0] = 0x7e;
	memcpy(outBuffer + 1, (char *)&opcode, 2);
	SWAP_WORD(outBuffer + 1);

	outBuffer[3] =  msgLen >> 8;
	outBuffer[4] =  msgLen;
	const char * pphone = Configure::GetInstance().Phone().c_str();
	for(int i = 0; i< 6; i++)
	{
		if( i==0 )
			outBuffer[5+i] = pphone[i*2]-'0';
		else
			outBuffer[5+i] = (pphone[i*2-1]-'0') << 4 | (pphone[i*2]-'0');
	}

	outBuffer[11] = seqno >> 8;
	outBuffer[12] = (unsigned char) seqno;
	return PROTOCOL_MESSAGE_HEADER_LENGTH + PROTOCOL_MESSAGE_BEGIN_LENGTH;
}

short JT808FormatProtocol::Crc8(const unsigned char * buf, const int len) {
	int result = 0;
	for (int i = 1; i < len; i++)
	{
		result ^= buf[i];
	}
	return result;
}

int JT808FormatProtocol::SWAP_WORD(unsigned char *buf)
{
	char  temp = buf[0];
	buf[0] = buf[1];
	buf[1] = temp ;
	return 0;
}

int JT808FormatProtocol::SWAP_DWORD(unsigned char *buf)
{
	char  temp = buf[0];
	buf[0] = buf[3];
	buf[3] = temp ;

	temp = buf[1] ;	
	buf[1] = buf[2];
	buf[2] =  temp;		
	return 0;
}

void JT808FormatProtocol::BCD_2_STRING(unsigned char *callnum,char *str, int n)
{
	BYTE high = 0;
	BYTE low = 0;

	for (int i = 0; i < n; i++)
	{
		high = 0x30|(callnum[i]>>4);
		low = 0x30|(callnum[i]&0x0f);
		str[2*i] = high;
		str[2*i+1] = low;
	}
}

void JT808FormatProtocol::STRING_2_BCD( char *str, unsigned char *callnum, int n)
{
	BYTE tmp;
	int i;
	for(i=0;i<n;i++)
	{ 
		tmp=str[2*i]-0x30;
		callnum[i]=tmp<<4|(str[2*i+1]-0x30)&0x0f;
	}
}

int JT808FormatProtocol::STRING_2_HEX(char *str)
{
	int sum = 0;
	for(int i = 0;str[i] != '\0';i++)
	{
		if(str[i]>='0' && str[i]<='9')
			sum = sum*16 + str[i]-'0';
		else if(str[i]<='f' && str[i]>='a')
			sum = sum*16 + str[i]-'a'+10;
		else if(str[i]<='F' && str[i]>='A')
			sum = sum*16 + str[i]-'A'+10;
	}
	return sum;
} 

void JT808FormatProtocol::ReturnToOriginal(const unsigned char *buf, int len,unsigned char *buf2,int len2) {
	int i=0,j=0;
	for( ;i<len && j<len2 ;i++,j++)
	{
		if(buf[i] == 0x7d)
		{
			if(buf[i+1] == 0x02)  
			{
				buf2[j] = 0x7e;
				i++;
			}
			else if(buf[i+1] == 0x01) 
			{
				buf2[j] = 0x7d;
				i++;
			}
			else 
			{
				buf2[j] = buf[i];	
			}
		}	
		else
			buf2[j] = buf[i];	
	}
}

int JT808FormatProtocol::OriginalToTransfer(const unsigned char *buf, int len,unsigned char *buf2,int len2) {
	buf2[0] = buf[0];
	int i = 1,j = 1;
	for( ; i < len - 1 && j < len2; i++, j++)
	{
		if(buf[i] == 0x7e)
		{
			buf2[j] = 0x7d;
			buf2[j+1] = 0x02;
			j++;
		}
		else if(buf[i] == 0x7d)
		{
			buf2[j] = 0x7d;
			buf2[j+1] = 0x01;
			j++;		
		}
		else
			buf2[j] = buf[i];
	}

	buf2[j] = buf[i];
	return j + 1;
}

const unsigned char* JT808FormatProtocol::Find(const unsigned char* buf, int len) {
	const byte* end_buf = buf + len;
	for ( ; buf != end_buf; ++buf) {
		if (*buf == 0x7e) return buf;
	}
	return NULL;
}

const unsigned char* JT808FormatProtocol::FindEx(const unsigned char* buf, int len) {
	for (int i = 1; i < len; ++i) {
		if (buf[i] == 0x7e) {
			if (i + 1 < kMsgMinLen) {
				return NULL;
			}
			else{
				return buf + i;
			}
		}
	}

	return NULL;
}

int JT808FormatProtocol::remote_char_converse_transfer(unsigned char *chr, unsigned int len) {
	unsigned int  i,j,k ;
	unsigned char *move;

	j=0 ;
	for (i=0; i<len; i++){
		if ( *chr++==0x7d ){
			j++;
			*(chr-1) = (*chr)^0x7c;

			move = chr;
			for (k=0; k<(len-i-2); k++){
				*move = *(move+1);
				move++;
			}
			*move = '\0';
		}
	}
	return (j);
}

int JT808FormatProtocol::UTF8toGBK(char *inbuf, char *outbuf, size_t* size) {
	return 0;
}

int JT808FormatProtocol::GBKtoUTF8(char *inbuf, char *outbuf, size_t* size) {
	return 0;
}
