#ifndef _HCASERVICE_JT808PROTOCOL_H
#define _HCASERVICE_JT808PROTOCOL_H

#include "Singleton.h"
#include "base_type.h"

//消息头长度
#define PROTOCOL_MESSAGE_BEGIN_LENGTH   1
#define PROTOCOL_MESSAGE_END_LENGTH     1
#define PROTOCOL_MESSAGE_CRC_LENGTH     1
#define PROTOCOL_MESSAGE_HEADER_LENGTH  12


#define PROTOCOL_MESSAGE_0200_LENGTH    46
#define PROTOCOL_MESSAGE_0704_LENGTH    51


enum {
	kError = -1,  // 出现错误
	kSuccess,     // 解析成功
	kNeedData     // 需要数据
};

typedef struct MSG_HEADER_ {
	unsigned short msgid;
	unsigned short seqno;
	unsigned short length;
	unsigned short encrypt;
	unsigned short package;
	unsigned char  bcd[6];
} MsgHeader;

typedef struct MSG_BODY_ {
	MsgHeader * header;
	unsigned char * content;
} MsgBody;

typedef struct MSG_PLATE_RESPONSE {
	MsgHeader * header;
	unsigned short reqno;
	unsigned short msgid;
	unsigned char  result;
} MsgPlateResponse;

typedef struct MSG_REGISTER_RESPONSE {
	MsgHeader * header;
	unsigned short reqno;
	unsigned char  result;
	std::string    authcode;
}MsgRegisterResponse;


class JT808FormatProtocol : public Singleton<JT808FormatProtocol>
{
public:
	JT808FormatProtocol(void);
	~JT808FormatProtocol(void);

public:
	int HandlePackageFrame(unsigned char * buf, int len, int & nIndex, int & nLength);

	int DecodeRegisterResponse(unsigned char * buf, int len, MsgRegisterResponse & msg);
	int DecodePlateformResponse(unsigned  char * buf, int len, MsgPlateResponse & msg);

	//encode request
	int Encode0x0001Response(unsigned char * outBuffer, int &nOutLen, unsigned short seqno, unsigned short opcode, unsigned char result);
	int Encode0200Request(Position * position, unsigned char * outBuffer, int &nOutLen, unsigned short seqno);
	int Encode0704Request(Position * position, unsigned char * outBuffer, int &nOutLen, unsigned short seqno);
	
	int EncodeAuthorRequest(unsigned char * outBuffer, int &nOutLen, std::string &authcode, unsigned short seqno);
	int EncodeRegisterRequest(unsigned char * outBuffer, int &nOutLen, unsigned short seqno);
	int EncodeHeartBeatRequest(unsigned char * outBuffer, int &nOutLen, unsigned short seqno);

	int Encode0x8104Response(unsigned char * outBuffer, int &nOutLen, unsigned short seqno);

    int DecodeMessageHeader(unsigned char * buff, int len, MsgHeader *header);
public:
	inline void ReturnToOriginal(const unsigned char *buf, int len,unsigned char *buf2,int len2);
	inline int OriginalToTransfer(const unsigned char *buf, int len,unsigned char *buf2,int len2);
	inline short Crc8(const unsigned char * buf, const int len);
	inline int SWAP_WORD(unsigned char *buf);
	inline int SWAP_DWORD(unsigned char *buf);
	inline void BCD_2_STRING(unsigned char *callnum,char *str, int n);
	inline void STRING_2_BCD(char *str, unsigned char *callnum, int n);
	inline int STRING_2_HEX(char *str);
	inline int UTF8toGBK(char *inbuf, char *outbuf, size_t* size);
	inline int GBKtoUTF8(char *inbuf, char *outbuf, size_t* size);


	const unsigned char* Find(const unsigned char* buf, int len);
	const unsigned char* FindEx(const unsigned char* buf, int len);
	int remote_char_converse_transfer(unsigned char *chr, unsigned int len);

private:
	inline int GeneralHeaderRequest(unsigned char * outBuffer, int &nInLen, int msgLen, unsigned short opcode, unsigned short seqno);

private:
	static const unsigned char kCmdLinkTestLen 	= 3;
	static const unsigned char kMsgMinLen      	= 15;
};
#endif /*_HCASERVICE_JT808PROTOCOL_H*/
