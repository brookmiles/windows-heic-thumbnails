#pragma once

void Log_Open(PCWSTR baseName);
void Log_Close();

void Log_Write(PCWSTR msg);
void Log_WriteFmt(PCWSTR fmt, ...);
