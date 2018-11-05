#include "Log.h"
#include <tchar.h>

HANDLE  hEventSource = NULL;

void LogInit(LPCTSTR lpSourceName)
{
    hEventSource = RegisterEventSource(NULL, lpSourceName);
}

void LogSucess(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    LPTSTR   lpszStrings[1];
    va_list  pArg;

    va_start(pArg, pFormat);
    _vstprintf_s(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;
    if (hEventSource != NULL)
    {
        ReportEvent(hEventSource, EVENTLOG_SUCCESS, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
}

extern void LogError(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    LPTSTR   lpszStrings[1];
    va_list  pArg;

    va_start(pArg, pFormat);
    _vstprintf_s(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;
    if (hEventSource != NULL)
    {
        ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
}

extern void LogWarning(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    LPTSTR   lpszStrings[1];
    va_list  pArg;

    va_start(pArg, pFormat);
    _vstprintf_s(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;
    if (hEventSource != NULL)
    {
        ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
}
extern void LogInfo(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    LPTSTR   lpszStrings[1];
    va_list  pArg;

    va_start(pArg, pFormat);
    _vstprintf_s(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;
    if (hEventSource != NULL)
    {
        ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
}