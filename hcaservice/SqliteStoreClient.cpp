#include "stdafx.h"

#include "SqliteStoreClient.h"
#include "Configure.h"
#include "Utils.h"
#include "MessageLocationWorker.h"
#include <windows.h>


SqliteStoreClient::SqliteStoreClient(void) 
	: m_maxRecord(0)
{
	m_sqliteDatabase = new CppSQLite3DB;
}


SqliteStoreClient::~SqliteStoreClient(void)
{
	CloseDataStore();
}


bool SqliteStoreClient::OpenDataStore() {
	std::string dbPath = Configure::GetInstance().ModulePath() + "position.db";
	std::string logPath = Configure::GetInstance().ModulePath() + "store_client.log";
	m_logger.Init(logPath.c_str(), 1024, 102400);
	
	try {
		m_sqliteDatabase->open(dbPath.c_str());
	} catch (CppSQLite3Exception* e) {
		m_logger.WriteError("err =[%s]\n", e->errorMessage());
		return false;
	}

	CheckExitsAndCreate();
	return true;
}


bool SqliteStoreClient::CloseDataStore() {
	if(m_sqliteDatabase != NULL) {
		try{
			m_sqliteDatabase->close();
			m_sqliteDatabase = NULL;
		}catch(CppSQLite3Exception * e) {
			m_logger.WriteError("err =[%s]\n", e->errorMessage());
		}
	}
	return true;
}

bool SqliteStoreClient::InsertData(PPosition position) {
	if(position == NULL) return false;
	char sqlBuffer[1024] = {0};

	sprintf_s(sqlBuffer, 1023, "insert into t_position(year,month,day,hour,minute,second,latitude,longitude,latdir,londir,starnum,state,diffst,speed,course) values (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", 
		position->year, position->month, position->day, position->hour, position->minute,position->second,position->latitude, position->longitude,
		position->latdir, position->londir, position->starnum, position->state, position->diff, position->speed, position->course);

	try {
		 m_sqliteDatabase->execDML(sqlBuffer);
	} catch (CppSQLite3Exception * e) {
		m_logger.WriteError("err =[%s]\n", e->errorMessage());
	}

	//m_logger.WriteInfo("sql =[%s] result=[%d]\n", sqlBuffer, result);
	return true;
}

bool SqliteStoreClient::CheckExitsAndCreate() {
	if(!m_sqliteDatabase->tableExists("t_position")) {
		m_sqliteDatabase->execDML("CREATE TABLE t_position (id INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1),year INTEGER,month INTEGER,day INTEGER,hour INTEGER,minute INTEGER,second INTEGER,latitude INTEGER,longitude INTEGER,latdir CHAR,londir CHAR,starnum INTEGER,state INTEGER,diffst INTEGER,speed INTEGER,course INTEGER)");
	}
	return true;
}

int SqliteStoreClient::LoadStorePosition() {
	CppSQLite3Query q = m_sqliteDatabase->execQuery("select count(*) from t_position");

	int count = 0;
	while (!q.eof()){
		count = q.getIntField(0);
		q.nextRow();
	}

	q.finalize();
	if(count == 0) return 0;
	q = m_sqliteDatabase->execQuery("select * from t_position order by id asc limit 0, 4 ");
	while (!q.eof()) {
		Position * position = new Position;
		position->type = OLD_TYPE;
		position->year = q.getIntField("year", 0);
		position->month = q.getIntField("month", 0);
		position->day = q.getIntField("day", 0);
		position->hour = q.getIntField("hour", 0);
		position->minute = q.getIntField("minute", 0);
		position->second = q.getIntField("second", 0);
		position->latitude = q.getIntField("latitude", 0);
		position->longitude = q.getIntField("longitude", 0);
		position->latdir = q.getIntField("latdir", 0);
		position->londir = q.getIntField("londir", 0);
		position->starnum = q.getIntField("starnum", 0);
		position->state = q.getIntField("state", 0);
		position->diff = q.getIntField("diffst", 0);
		position->speed = q.getIntField("speed", 0);
		position->course = q.getIntField("course", 0);
		m_maxRecord = q.getIntField("id", 0);
		MessageLocationWorker::GetInstance().PushPosition(position);
		q.nextRow();
	}

	q.finalize();
	char sqlBuffer[128] = {0};
	sprintf_s(sqlBuffer, 127, "delete from t_position where id <= %d", m_maxRecord);
	m_sqliteDatabase->execDML(sqlBuffer);
	return count;
}

int SqliteStoreClient::ClearStorePosition() {
	m_sqliteDatabase->execDML("delete from t_position");
	return 1;
}

