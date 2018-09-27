
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
using namespace std;


//定义全局函数变量
void Init();
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
void LogEvent(LPCTSTR pszFormat, ...);
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

TCHAR szServiceName[] = _T("ztAutoService");
BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	Init();

	dwThreadID = ::GetCurrentThreadId();

	SERVICE_TABLE_ENTRY st[] =
	{
		{ szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (_stricmp(lpCmdLine, "-i") == 0)
	{
		Install();
	}
	else if (_stricmp(lpCmdLine, "-u") == 0)
	{
		Uninstall();
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(st))
		{
			LogEvent(_T("Register Service Main Function Error!"));
		}
	}

	return 0;
}
//*********************************************************
//Functiopn:            Init
//Description:          初始化
//Calls:                main
//*********************************************************
void Init()
{
	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
}

//*********************************************************
//Functiopn:            ServiceMain
//Description:          服务主函数，这在里进行控制对服务控制的注册
//*********************************************************
void WINAPI ServiceMain()
{
	// Register the control request handler
	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//注册服务控制
	hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);
	if (hServiceStatus == NULL)
	{
		LogEvent(_T("Handler not installed"));
		return;
	}
	SetServiceStatus(hServiceStatus, &status);

	status.dwWin32ExitCode = S_OK;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &status);

	//服务执行内容
	TCHAR szFilePath[MAX_PATH] = { 0 };
	TCHAR szDrive[_MAX_DRIVE]  = { 0 };
	TCHAR szDir[_MAX_DIR]      = { 0 };
	TCHAR szFname[_MAX_FNAME]  = { 0 };
	TCHAR szExt[_MAX_EXT]      = { 0 };
	TCHAR szConfPath[MAX_PATH] = { 0 };

	::GetModuleFileName(NULL, szFilePath, MAX_PATH);
	_wsplitpath_s(szFilePath, szDrive, szDir, szFname, szExt);
	wsprintf(szConfPath, _T("%s%s\\config.ini"), szDrive, szDir);
	LogEvent(_T("config file: %s"), szConfPath);

	TCHAR szNumber[20];
	GetPrivateProfileString(_T("common"), _T("number"), _T("1"), szNumber, 20, szConfPath);
	LogEvent(_T("task num: %s"), szNumber);
	int nNumber = _wtoi(szNumber);
	for (int i=0; i<nNumber; i++)
	{
		TCHAR szApp[MAX_PATH] = { 0 };
		TCHAR szPath[MAX_PATH] = { 0 };
		TCHAR szCmd[MAX_PATH] = { 0 };
		wsprintf(szApp, _T("file%d"), i + 1);
		GetPrivateProfileString(szApp, _T("path"), _T("exe"), szPath, MAX_PATH - 1, szConfPath);
		GetPrivateProfileString(szApp, _T("cmd"), _T(""), szCmd, MAX_PATH - 1, szConfPath);

		TCHAR szRun[MAX_PATH * 2] = { 0 };
		wsprintf(szRun, _T("%s %s"), szPath, szCmd);
		//LogEvent(_T("execute: %s"), szRun);
  //      int ret = _wsystem(szRun);
        /*UINT ret = WinExec((LPCSTR)szRun, SW_NORMAL);*/
        //LogEvent(_T("execute finish %d"), ret);

		STARTUPINFO         si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(si);
		GetStartupInfo(&si);
		si.wShowWindow = SW_SHOW;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		if (!CreateProcess(NULL, szRun, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			DWORD dwerrcode = GetLastError();
			LogEvent(_T("CreateProcess error:%d"), dwerrcode);
		}
		LogEvent(_T("execute finish"));
	}
	while(status.dwCurrentState == SERVICE_RUNNING)
		Sleep(10000);
	//服务执行内容

	//执行结束
	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &status);
	LogEvent(_T("Service stopped"));
}

//*********************************************************
//Functiopn:            ServiceStrl
//Description:          服务控制主函数，这里实现对服务的控制，
//                      当在服务管理器上停止或其它操作时，将会运行此处代码
//*********************************************************
void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus, &status);
		PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		LogEvent(_T("Bad service request"));
	}
}

//*********************************************************
//Functiopn:            IsInstalled
//Description:          判断服务是否已经被安装
//*********************************************************
BOOL IsInstalled()
{
	BOOL bResult = FALSE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

//*********************************************************
//Functiopn:            Install
//Description:          安装服务函数
//*********************************************************
BOOL Install()
{
	if (IsInstalled())
		return TRUE;

	//打开服务控制管理器
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务
	SC_HANDLE hService = ::CreateService(
		hSCM, szServiceName, szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, _T(""), NULL, NULL);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
		return FALSE;
	}

	//启动系统服务
	if (StartService(hService, 0, NULL) == false)
	{
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCM);
		return FALSE;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}

//*********************************************************
//Functiopn:            Uninstall
//Description:          删除服务函数
//*********************************************************
BOOL Uninstall()
{
	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务
	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	if (bDelete)
		return TRUE;

	LogEvent(_T("Service could not be deleted"));
	return FALSE;
}

//*********************************************************
//Functiopn:            LogEvent
//Description:          记录服务事件
//*********************************************************
void LogEvent(LPCTSTR pFormat, ...)
{
	TCHAR    chMsg[256];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	va_list pArg;

	va_start(pArg, pFormat);
	_vstprintf_s(chMsg, pFormat, pArg);
	va_end(pArg);

	lpszStrings[0] = chMsg;

	hEventSource = RegisterEventSource(NULL, szServiceName);
	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
		DeregisterEventSource(hEventSource);
	}
}
