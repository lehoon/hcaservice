#ifndef _HCASERVICE_LOGGER_H_
#define _HCASERVICE_LOGGER_H_

#include <stdio.h>

class Logger
{
public:
	Logger(void);
	~Logger(void);

public:
	bool Init(const char* file_name, int log_buf_size = 8192, int split_lines = 5000000);
	void Write(int level, const char* format, ...);
	void WriteError(const char* format, ...);
	void WriteInfo(const char* format, ...);
	void WriteDebug(const char* format, ...);
	void WriteWarn(const char* format, ...);
	void Flush(void);

private:
	int Gettimeofday(struct timeval *tv, void * tzp);

private:
	char		m_dirname[128];
	char		m_logname[128];
	int			m_split_lines;
	int			m_log_buf_size;
	long long	m_count;
	int			m_today;
	FILE	*	m_fp;
	char	*	m_buf;
};

#endif /*_HCASERVICE_LOGGER_H_*/
