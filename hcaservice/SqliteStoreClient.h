#ifndef _HCASERVICE_SQLITESTORE_CLINET_H_
#define _HCASERVICE_SQLITESTORE_CLINET_H_

#include "base_type.h"
#include "sqlite3.h"
#include "CppSQLite3.h"
#include "Singleton.h"
#include "Logger.h"

class SqliteStoreClient : public Singleton<SqliteStoreClient>
{
public:
	SqliteStoreClient(void);
	~SqliteStoreClient(void);

public:
	bool OpenDataStore();
	bool CloseDataStore();
	int  LoadStorePosition();
	int  ClearStorePosition();
	bool InsertData(PPosition position);

private:
	bool CheckExitsAndCreate();

private:
	CppSQLite3DB *  m_sqliteDatabase;
	Logger          m_logger;
	unsigned int    m_maxRecord;
};

#endif /*_HCASERVICE_SQLITESTORE_CLINET_H_*/
