#pragma once
#include <windows.h>

extern void LogInit(LPCTSTR);
extern void LogSucess(LPCTSTR pFormat, ...);
extern void LogError(LPCTSTR pFormat, ...);
extern void LogWarning(LPCTSTR pFormat, ...);
extern void LogInfo(LPCTSTR pFormat, ...);
