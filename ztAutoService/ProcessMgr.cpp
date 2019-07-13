#include "ProcessMgr.h"
#include "Log.h"
#include <windows.h>
#include <time.h>
#include <thread>
#include <tchar.h>

CProcessMgr::CProcessMgr(void)
{
}


CProcessMgr::~CProcessMgr(void)
{
}

void CProcessMgr::Start()
{
    // 配置文件路径
    TCHAR szFilePath[MAX_PATH] = { 0 };
    TCHAR szDrive[_MAX_DRIVE]  = { 0 };
    TCHAR szDir[_MAX_DIR]      = { 0 };
    TCHAR szFname[_MAX_FNAME]  = { 0 };
    TCHAR szExt[_MAX_EXT]      = { 0 };
    TCHAR szConfPath[MAX_PATH] = { 0 };

    ::GetModuleFileName(NULL, szFilePath, MAX_PATH);
    _wsplitpath_s(szFilePath, szDrive, szDir, szFname, szExt);
    wsprintf(szConfPath, _T("%s%sconfig.ini"), szDrive, szDir);
    //LogInfo(_T("config file: %s"), szConfPath);

    //配置内容
    int nNumber = GetPrivateProfileInt(_T("common"), _T("number"), 0, szConfPath);
    for (int i=0; i<nNumber; i++)
    {
        ProcessInfo* newProcess = new ProcessInfo;
        if(nullptr == newProcess) return;
        ZeroMemory(newProcess->szPath,MAX_PATH);
        ZeroMemory(newProcess->szCmd,MAX_PATH);

        TCHAR szApp[MAX_PATH] = { 0 };
        wsprintf(szApp, _T("file%d"), i + 1);

        GetPrivateProfileString(szApp, _T("path"), _T(""), newProcess->szPath, MAX_PATH - 1, szConfPath);
        GetPrivateProfileString(szApp, _T("cmd"), _T(""), newProcess->szCmd, MAX_PATH - 1, szConfPath);
        INT nProtect = GetPrivateProfileInt(szApp, _T("protect"), 0, szConfPath);
        INT nRestartHour = GetPrivateProfileInt(szApp, _T("rshour"), -1, szConfPath);
        INT nRestartDur = GetPrivateProfileInt(szApp, _T("rsdur"), 0, szConfPath);

        newProcess->lPID = 0;
        newProcess->bProtect = nProtect != 0;
        newProcess->nReStartHour = nRestartHour;
        newProcess->nRsStartDur = nRestartDur;

        m_vecProcess.push_back(newProcess);
        //LogInfo(_T("execute:%s finish"),szPath);
    }
}

void CProcessMgr::Stop()
{
    for (auto& pProcess:m_vecProcess)
    {
        if(pProcess->lPID > 0)
        {
            Kill(pProcess->lPID);
        }
    }
}

bool CProcessMgr::ProtectRun()
{
    struct tm timeinfo;
    time_t now = time(NULL);
    localtime_s(&timeinfo, &now);
    for (auto& pProcess:m_vecProcess)
    {
        if ( pProcess->lPID == 0                        //< 尚未启动
            || (pProcess->bProtect
                && (!Find(pProcess->lPID)                    //< 进程异常退出
                    || (pProcess->nReStartHour >= 0
                        && pProcess->nReStartHour <= 23
                        && difftime(now,pProcess->nStartTime) > 3600   //< 60*60
                        && timeinfo.tm_hour == pProcess->nReStartHour)  //< 重启时间
                    || (pProcess->nRsStartDur > 0
                        && difftime(now,pProcess->nStartTime) > pProcess->nRsStartDur*216000) //运行时间
                    )
                )
            )
        {
            // 重启程序
            std::thread t([&](){
                try
                {
                    RunChild(pProcess);
                }
                catch(...)
                {
                    LogError(_T("RunChild error"));
                }
            });
            t.detach();
        }
    }
    return true;
}

bool CProcessMgr::RunChild(ProcessInfo* pro)
{
    if(pro->lPID != 0 && Find(pro->lPID) && !Kill(pro->lPID))
    {
        LogError(_T("kill process:%d failed"), pro->lPID);
        return false;
    }

    if(!CreateChildProcess(pro->szPath, pro->szCmd, pro->lPID))
    {
        LogError(_T("restart process failed"));
        return false;
    }

    pro->nStartTime = time(NULL);

    return true;
}

bool CProcessMgr::CreateChildProcess(PTCHAR szPath, PTCHAR szCmd, DWORD& lPID)
{
    bool bRes = false;

    /** 创建管道，重定向子进程标准输入 */
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE childStdInRead = INVALID_HANDLE_VALUE, 
        childStdInWrite = INVALID_HANDLE_VALUE;

    CreatePipe(&childStdInRead, &childStdInWrite, &sa, 0);
    SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0);

    /** 创建子进程 */
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = childStdInRead;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_MINIMIZE;
    si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    pi.hProcess = INVALID_HANDLE_VALUE;
    pi.hThread = INVALID_HANDLE_VALUE;

    TCHAR szRun[MAX_PATH * 2] = { 0 };
    wsprintf(szRun, _T("%s %s"), szPath, szCmd);

    if (!CreateProcess(NULL, szRun, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
    {
        LogError(_T("CreateProcess failed:%d"), GetLastError());
        goto end;
    }
    lPID = pi.dwProcessId;
    bRes = true;

end:
    if(INVALID_HANDLE_VALUE != childStdInWrite)
        CloseHandle(childStdInWrite);
    if(INVALID_HANDLE_VALUE != pi.hProcess)
        CloseHandle(pi.hProcess);
    if(INVALID_HANDLE_VALUE != pi.hThread)
        CloseHandle(pi.hThread);

    if(bRes)
        LogSucess(_T("RunChild:%s, PID:%ld success"),szRun,pi.dwProcessId);
    else
        LogError(_T("RunChild:%s, PID:%ld failed"),szRun,pi.dwProcessId);

    return bRes;
}

bool CProcessMgr::Find(DWORD lPID)
{
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS,FALSE,lPID);
    if (NULL == h)
    {
        LogError(_T("unfind process PID:%ld"),lPID);
        return false;
    }
    CloseHandle(h);
    return true;
}

bool CProcessMgr::Kill(DWORD lPID)
{
    int i = 5;
    //Log::debug("begin kill PID:%ld",lPID);
    while(i--)
    {
        HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,lPID);
        if(NULL == h)
        {
            printf("kill process %ld sucess\r\n", lPID);
            return true;
        }
        if(FALSE == TerminateProcess(h,0))
        {
            DWORD dwError = GetLastError();
            printf("TerminateProcess failed:%d\r\n",dwError);
        }
        CloseHandle(h);
        Sleep(100);
    }
    //Log::debug("end kill PID:%ld",lPID);
    HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,lPID);
    if(NULL == h)
    {
        printf("kill process %ld sucess\r\n", lPID);
        return true;
    }

    CloseHandle(h);
    printf("process is still exist:%ld(handle:%d)\r\n", lPID,h);
    return false;
}