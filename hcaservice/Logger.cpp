#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Logger.h"

Logger::Logger(void)
{
	m_count = 0;
	memset(m_dirname, 0, sizeof(m_dirname));
	memset(m_logname, 0, sizeof(m_logname));
}

Logger::~Logger(void)
{
	if(m_fp != NULL)
	{
		fclose(m_fp);
		m_fp = NULL;
	}

	if(m_buf != NULL)
	{
		delete [] m_buf;
		m_buf = NULL;
	}
}

bool Logger::Init(const char* file_name, int log_buf_size /* = 8192 */, int split_lines /* = 5000000 */)
{
	m_log_buf_size = log_buf_size;
	m_split_lines = split_lines;
	m_buf = new char[m_log_buf_size];
	memset(m_buf, 0, sizeof(m_buf));

	time_t t = time(NULL);
	struct tm mytm;
	localtime_s(&mytm, &t);
	char  * pfilename = (char *) file_name;
	char * p = strrchr(pfilename, '\\');

	char log_full_name[256] = {0};
	if (p == NULL)
	{
		strncpy_s(m_logname, file_name, sizeof(m_logname));
		_snprintf(log_full_name, 255, "%d_%02d_%02d_%s", mytm.tm_year + 1900, mytm.tm_mon  + 1, mytm.tm_mday, file_name);
	}
	else
	{
		strncpy_s(m_logname, p + 1, sizeof(m_logname));
		strncpy_s(m_dirname, file_name, p - file_name + 1);
		_snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", m_dirname, mytm.tm_year + 1900, mytm.tm_mon  + 1, mytm.tm_mday, m_logname);
	}

	m_today = mytm.tm_mday;
	m_fp = fopen(log_full_name, "a+");

	if (m_fp == NULL)
	{
		return false;
	}

	return true;
}

void Logger::Write(int level, const char* format, ...)
{
	struct timeval now = {0, 0};
	Gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm  mytm;
	localtime_s(&mytm, &t);
	char s[16] = {0};

	switch (level)
	{
	case 0: strncpy(s, "[debug]", 15);
		break;
	case 1: strncpy(s, "[info]", 15);
		break;
	case 2: strncpy(s, "[warn]", 15);
		break;
	case 3: strncpy(s, "[error]", 15);
		break;
	default: strncpy(s, "[info]", 15);
		break;
	}

	int n = _snprintf(m_buf, 48, "%s [%d-%02d-%02d %02d:%02d:%02d.%06d] ",  s, mytm.tm_year+1900, mytm.tm_mon+1, mytm.tm_mday,
		mytm.tm_hour, mytm.tm_min, mytm.tm_sec, now.tv_usec);

	m_count++;
	if(m_today  != mytm.tm_mday || m_count % m_split_lines == 0)
	{
		char newfile[256] = {0};
		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;


		char filetail[16] = {0};
		_snprintf(filetail, 16, "%d_%02d_%02d_", mytm.tm_year + 1900, mytm.tm_mon + 1, mytm.tm_mday);

		if(m_today != mytm.tm_mday) {
			_snprintf(newfile, 255, "%s%s%s", m_dirname, filetail, m_logname);
			m_today = mytm.tm_mday;
			m_count = 0;
		} else {
			_snprintf(newfile, 255, "%s%s%s.%d", m_dirname, filetail, m_logname, m_count / m_split_lines);
		}

		m_fp = fopen(newfile, "a+");
	}

	va_list valist;
	va_start(valist, format);
	int m = _vsnprintf(m_buf + n, m_log_buf_size - 1, format, valist);
	m_buf[n + m + 1] = '\n';
	fwrite(m_buf, strlen(m_buf), 1, m_fp);
	va_end(valist);
}

void Logger::WriteError(const char* format, ...)
{
	struct timeval now = {0, 0};
	Gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm * sys_tm = localtime(&t);
	struct tm  mytm = *sys_tm;

	int n = _snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d [error] ", mytm.tm_year+1900, mytm.tm_mon+1, mytm.tm_mday,
		mytm.tm_hour, mytm.tm_min, mytm.tm_sec, now.tv_usec);

	m_count++;
	if(m_today  != mytm.tm_mday || m_count % m_split_lines == 0)
	{
		char newfile[256] = {0};
		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;

		char filetail[16] = {0};
		_snprintf(filetail, 16, "%d_%02d_%02d_", mytm.tm_year + 1900, mytm.tm_mon + 1, mytm.tm_mday);

		if(m_today != mytm.tm_mday)
		{
			_snprintf(newfile, 255, "%s%s%s", m_dirname, filetail, m_logname);
			m_today = mytm.tm_mday;
			m_count = 0;
		}
		else
		{
			_snprintf(newfile, 255, "%s%s%s.%d", m_dirname, filetail, m_logname, m_count / m_split_lines);
		}

		m_fp = fopen(newfile, "a+");
	}

	va_list valist;
	va_start(valist, format);
	int m = _vsnprintf(m_buf + n, m_log_buf_size - 1, format, valist);
	m_buf[n + m + 1] = '\n';
	fwrite(m_buf, strlen(m_buf), 1, m_fp);
	va_end(valist);
}

void Logger::WriteInfo(const char* format, ...)
{
	struct timeval now = {0, 0};
	Gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm * sys_tm = localtime(&t);
	struct tm  mytm = *sys_tm;

	int n = _snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d [info] ", mytm.tm_year+1900, mytm.tm_mon+1, mytm.tm_mday,
		mytm.tm_hour, mytm.tm_min, mytm.tm_sec, now.tv_usec);

	m_count++;
	if(m_today  != mytm.tm_mday || m_count % m_split_lines == 0)
	{
		char newfile[256] = {0};
		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;

		char filetail[16] = {0};
		_snprintf(filetail, 16, "%d_%02d_%02d_", mytm.tm_year + 1900, mytm.tm_mon + 1, mytm.tm_mday);

		if(m_today != mytm.tm_mday)
		{
			_snprintf(newfile, 255, "%s%s%s", m_dirname, filetail, m_logname);
			m_today = mytm.tm_mday;
			m_count = 0;
		}
		else
		{
			_snprintf(newfile, 255, "%s%s%s.%d", m_dirname, filetail, m_logname, m_count / m_split_lines);
		}

		m_fp = fopen(newfile, "a+");
	}

	va_list valist;
	va_start(valist, format);
	int m = _vsnprintf(m_buf + n, m_log_buf_size - 1, format, valist);
	m_buf[n + m + 1] = '\n';
	fwrite(m_buf, strlen(m_buf), 1, m_fp);
	va_end(valist);
}

void Logger::WriteDebug(const char* format, ...)
{
	struct timeval now = {0, 0};
	Gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm * sys_tm = localtime(&t);
	struct tm  mytm = *sys_tm;

	int n = _snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d [debug] ", mytm.tm_year+1900, mytm.tm_mon+1, mytm.tm_mday,
		mytm.tm_hour, mytm.tm_min, mytm.tm_sec, now.tv_usec);

	m_count++;
	if(m_today  != mytm.tm_mday || m_count % m_split_lines == 0)
	{
		char newfile[256] = {0};
		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;

		char filetail[16] = {0};
		_snprintf(filetail, 16, "%d_%02d_%02d_", mytm.tm_year + 1900, mytm.tm_mon + 1, mytm.tm_mday);

		if(m_today != mytm.tm_mday)
		{
			_snprintf(newfile, 255, "%s%s%s", m_dirname, filetail, m_logname);
			m_today = mytm.tm_mday;
			m_count = 0;
		}
		else
		{
			_snprintf(newfile, 255, "%s%s%s.%d", m_dirname, filetail, m_logname, m_count / m_split_lines);
		}

		m_fp = fopen(newfile, "a+");
	}

	va_list valist;
	va_start(valist, format);

	int m = _vsnprintf(m_buf + n, m_log_buf_size - 1, format, valist);
	m_buf[n + m + 1] = '\n';
	fwrite(m_buf, strlen(m_buf), 1, m_fp);
	va_end(valist);
}

void Logger::WriteWarn(const char* format, ...)
{
	struct timeval now = {0, 0};
	Gettimeofday(&now, NULL);
	time_t t = now.tv_sec;
	struct tm * sys_tm = localtime(&t);
	struct tm  mytm = *sys_tm;

	int n = _snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06d [warn] ", mytm.tm_year+1900, mytm.tm_mon+1, mytm.tm_mday,
		mytm.tm_hour, mytm.tm_min, mytm.tm_sec, now.tv_usec);

	m_count++;
	if(m_today  != mytm.tm_mday || m_count % m_split_lines == 0)
	{
		char newfile[256] = {0};
		fflush(m_fp);
		fclose(m_fp);
		m_fp = NULL;

		char filetail[16] = {0};
		_snprintf(filetail, 16, "%d_%02d_%02d_", mytm.tm_year + 1900, mytm.tm_mon + 1, mytm.tm_mday);

		if(m_today != mytm.tm_mday)
		{
			_snprintf(newfile, 255, "%s%s%s", m_dirname, filetail, m_logname);
			m_today = mytm.tm_mday;
			m_count = 0;
		}
		else
		{
			_snprintf(newfile, 255, "%s%s%s.%d", m_dirname, filetail, m_logname, m_count / m_split_lines);
		}

		m_fp = fopen(newfile, "a+");
	}

	va_list valist;
	va_start(valist, format);
	int m = _vsnprintf(m_buf + n, m_log_buf_size - 1, format, valist);
	m_buf[n + m + 1] = '\n';
	fwrite(m_buf, strlen(m_buf), 1, m_fp);
	va_end(valist);
}

void Logger::Flush()
{
	
	fflush(m_fp);
	
}

int Logger::Gettimeofday(struct timeval *tv, void * tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon  = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min  = wtm.wMinute;
	tm.tm_sec  = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv->tv_sec = (long) clock;
	tv->tv_usec = wtm.wMilliseconds * 1000;
	return 0;
}