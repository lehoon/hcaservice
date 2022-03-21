#ifndef _HCASERVICE_CONFIGURE_H
#define _HCASERVICE_CONFIGURE_H

#include "stdafx.h"
#include "copy_disable.h"
#include "Singleton.h"

class Configure : public Singleton<Configure>
{
public:
	Configure(void);
	~Configure(void);

public:
	void SetModulePath(std::string & path);
	void SetRemoteHost(std::string & host);
	void SetRemotePort(const unsigned int port);

	void SetStationHost(std::string & host);
	void SetStationPort(const unsigned int port);
	void SetStationMountPoint(std::string & mountpoint);
	void SetStationUsername(std::string & username);
	void SetStationPassword(std::string & password);

	void SetSerialPortNo(const unsigned int no);
	void SetSerialPortBaud(const unsigned int baud);

	void SetPhone(const char * phone);
	void SetPhone(std::string & phone);
	void SetNumberPlate(std::string & numberplate);
	void SetManuFacturer(std::string & manufacturer);
	void SetModel(std::string & model);
	void SetUnitNo(std::string & unitno);
	void SetProvince(unsigned short privince);
	void SetCity(unsigned short city);
	void SetColor(const byte color);
	void SetNeedRegister(bool need);
	void SetPurchasers(unsigned int purchasers);

	//==================================
	std::string & ModulePath() ;
	std::string & RemoteHost() ;
	unsigned int RemotePort() ;

	std::string & StationHost() ;
	unsigned int StationPort() ;
	std::string & StationMountPoint() ;
	std::string & StationUsername() ;
	std::string & StationPassword() ;

	unsigned int SerialPortNo() ;
	unsigned int SerialPortBaud() ;

	std::string & Phone();
	std::string & NumberPlate() ;
	std::string & ManuFacturer();
	std::string & Model() ;
	std::string & UnitNo() ;
	unsigned short Province();
	unsigned short City();
	byte Color() ;
	bool NeedRegister();
    unsigned int purchasers();

private:
	//程序路径
	std::string   m_modulePath;
	//配置文件项
	std::string   m_remote_host;
	unsigned int  m_remote_port;

	//基站配置
	std::string m_station_host;
	unsigned int m_station_port;
	std::string m_station_mountpoint;
	std::string m_station_username;
	std::string m_station_password;

	//串口配置
	unsigned int m_serial_no;
	unsigned int m_serial_baud;

	//车辆配置
	std::string m_numberplate;
	std::string m_phone;
	unsigned short m_privince;
	unsigned short m_city;
	std::string m_manufacturer;
	std::string m_model;
	std::string m_unitno;
	byte m_color;

	bool  m_needregister;
	unsigned int m_purchasers;

	DISABLE_COPY_AND_MOVE(Configure);
};

#endif /*_HCASERVICE_BASE_TYPES_H*/
