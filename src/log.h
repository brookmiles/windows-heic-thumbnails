#pragma once

enum LOG_LEVEL
{
	LOG_NONE,

	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
	LOG_TRACE,

	LOG_MAX,
};

void Log_SetLevel(LOG_LEVEL lvl);

void Log_Open(PCWSTR baseName);
void Log_Close();

void Log_Write(LOG_LEVEL lvl, PCWSTR msg);
void Log_WriteFmt(LOG_LEVEL lvl, PCWSTR fmt, ...);
