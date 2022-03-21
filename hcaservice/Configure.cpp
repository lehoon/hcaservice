#include "stdafx.h"
#include "Configure.h"

Configure::Configure(void)
{
}


Configure::~Configure(void)
{
}

void Configure::SetModulePath(std::string & path){
	m_modulePath = path;
}

void Configure::SetRemoteHost(std::string & host){
	m_remote_host = host;
}

void Configure::SetRemotePort(const unsigned int port){
	m_remote_port = port;
}

void Configure::SetStationHost(std::string & host){
	m_station_host = host;
}

void Configure::SetStationPort(const unsigned int port){
	m_station_port = port;
}

void Configure::SetStationMountPoint(std::string & mountpoint){
	m_station_mountpoint = mountpoint;
}

void Configure::SetStationUsername(std::string & username){
	m_station_username = username;
}

void Configure::SetStationPassword(std::string & password){
	m_station_password = password;
}

void Configure::SetSerialPortNo(const unsigned int no){
	m_serial_no = no;
}

void Configure::SetSerialPortBaud(const unsigned int baud){
	m_serial_baud = baud;
}

void Configure::SetPhone(const char * phone) {
	m_phone = phone;
}

void Configure::SetPhone(std::string & phone){
	m_phone = phone;
}

void Configure::SetNumberPlate(std::string & numberplate){
	m_numberplate = numberplate;
}
void Configure::SetManuFacturer(std::string & manufacturer){
	m_manufacturer = manufacturer;
}

void Configure::SetModel(std::string & model){
	m_model = model;
}

void Configure::SetUnitNo(std::string & unitno){
	m_unitno = unitno;
}

void Configure::SetProvince(unsigned short privince){
	m_privince = privince;
}

void Configure::SetCity(unsigned short city){
	m_city = city;
}

void Configure::SetColor(const byte color){
	m_color = color;
}

void Configure::SetNeedRegister(bool need) {
	m_needregister = need;
}

std::string & Configure::ModulePath() {
	return m_modulePath;
}

std::string & Configure::RemoteHost() {
	return m_remote_host;
}

unsigned int Configure::RemotePort() {
	return m_remote_port;
}

std::string & Configure::StationHost() {
	return m_station_host;
}

unsigned int Configure::StationPort() {
	return m_station_port;
}

std::string & Configure::StationMountPoint() {
	return m_station_mountpoint;
}

std::string & Configure::StationUsername() {
	return m_station_username;
}

std::string & Configure::StationPassword() {
	return m_station_password;
}

unsigned int Configure::SerialPortNo() {
	return m_serial_no;
}

unsigned int Configure::SerialPortBaud() {
	return m_serial_baud;
}

std::string & Configure::Phone(){
	return m_phone;
}

std::string & Configure::NumberPlate()  {
	return m_numberplate;
}

std::string & Configure::ManuFacturer()  {
	return m_manufacturer;
}

std::string & Configure::Model() {
	return m_model;
}

std::string & Configure::UnitNo() {
	return m_unitno;
}

unsigned short Configure::Province() {
	return m_privince;
}

unsigned short Configure::City() {
	return m_city;
}

byte Configure::Color() {
	return m_color;
}

void Configure::SetPurchasers(unsigned int purchasers) {
	m_purchasers = purchasers;
}

unsigned int Configure::purchasers() {
	return m_purchasers;
}
