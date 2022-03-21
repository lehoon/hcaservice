#ifndef _HCASERVICE_SCHEDULE_TASK_H
#define _HCASERVICE_SCHEDULE_TASK_H

#include "IWorkThread.h"

class CScheduleTaskWorker : public IThreadUser
{
public:
	CScheduleTaskWorker(void);
	~CScheduleTaskWorker(void);

public:
	bool InitTask();
	bool ExitTask();
	bool Work();


public:
	bool DeleteLogFile();

};

#endif /*_HCASERVICE_SCHEDULE_TASK_H*/
