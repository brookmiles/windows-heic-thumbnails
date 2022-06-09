#include <objbase.h>
#include <shlobj_core.h>
#include <strsafe.h>
#include <pathcch.h>


#include "log.h"

PCWSTR LOG_EXT = L".log";
size_t path_extra_cch = 10; // extension + "\" + good luck

HANDLE hLog = INVALID_HANDLE_VALUE;

PWSTR GetFullLogPath(PCWSTR baseName)
{
	PWSTR result_path = NULL;

	size_t base_name_len = 0;
	HRESULT hr = StringCchLength(baseName, STRSAFE_MAX_CCH, &base_name_len);
	if (SUCCEEDED(hr))
	{
		PWSTR app_local_path = 0;
		hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &app_local_path);
		if (SUCCEEDED(hr))
		{
			size_t app_local_len = 0;
			hr = StringCchLength(app_local_path, STRSAFE_MAX_CCH, &app_local_len);
			if (SUCCEEDED(hr))
			{
				size_t full_path_cch = app_local_len + base_name_len + path_extra_cch;
				PWSTR full_path_buf = (PWSTR)LocalAlloc(LPTR, full_path_cch * sizeof(WCHAR));
				if (full_path_buf)
				{
					hr = PathCchCombineEx(full_path_buf, full_path_cch, app_local_path, baseName, PATHCCH_ALLOW_LONG_PATHS);
					if (SUCCEEDED(hr))
					{
						hr = StringCchCat(full_path_buf, full_path_cch, LOG_EXT);
						if (SUCCEEDED(hr))
						{
							result_path = full_path_buf;
							full_path_buf = NULL;
						}
					}
					// full_path_buf will still be set if something failed
					LocalFree(full_path_buf);
					full_path_buf = NULL;
				}
			}
		}
		CoTaskMemFree(app_local_path);
	}

	return result_path;
}

void Log_Open(PCWSTR baseName)
{
	if (hLog != INVALID_HANDLE_VALUE)
		return;

	PWSTR log_file_path = GetFullLogPath(baseName);
	if (log_file_path)
	{
		hLog = CreateFile(log_file_path, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hLog != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hLog, 0, 0, FILE_END);

			//WCHAR test[] = L"Hello!\r\n";
			//DWORD dwToWrite = (DWORD)(wcslen(test) * sizeof(WCHAR));
			//DWORD dwWritten = 0;
			//WriteFile(hLog, test, dwToWrite, &dwWritten, NULL);
		}
		LocalFree(log_file_path);
	}
}

void Log_Close()
{
	if (hLog != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hLog);
		hLog = INVALID_HANDLE_VALUE;
	}
}

void Write_String(PCWSTR msg)
{
	size_t msg_len = 0;
	HRESULT hr = StringCchLength(msg, STRSAFE_MAX_CCH, &msg_len);
	if (SUCCEEDED(hr))
	{
		DWORD dwToWrite = (DWORD)(msg_len * sizeof(WCHAR));
		DWORD dwWritten = 0;
		WriteFile(hLog, msg, dwToWrite, &dwWritten, NULL);
	}
}

const UINT DATE_STAMP_CCH = 50;
WCHAR date_stamp_buf[DATE_STAMP_CCH] = {};

const UINT TIME_STAMP_CCH = 50;
WCHAR time_stamp_buf[TIME_STAMP_CCH] = {};

void Log_Write(PCWSTR msg)
{
	if (hLog == INVALID_HANDLE_VALUE)
		return;

	SYSTEMTIME date_time;
	GetLocalTime(&date_time);

	int date_fmt_res = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &date_time, NULL, date_stamp_buf, DATE_STAMP_CCH, NULL);
	if (date_fmt_res > 0)
	{
		Write_String(date_stamp_buf);
		Write_String(L" ");
	}

	int time_fmt_res = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, &date_time, NULL, time_stamp_buf, TIME_STAMP_CCH);
	if (time_fmt_res > 0)
	{
		Write_String(time_stamp_buf);
		Write_String(L" ");
	}

	Write_String(msg);
	Write_String(L"\r\n");
}

const UINT LOG_FMT_CCH = 1024;
WCHAR fmt_log_buf[LOG_FMT_CCH] = {};

void Log_WriteFmt(PCWSTR fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	HRESULT hr = StringCchVPrintfW(fmt_log_buf, LOG_FMT_CCH, fmt, args );
	va_end(args);

	if (SUCCEEDED(hr))
	{
		Log_Write(fmt_log_buf);
	}
}
