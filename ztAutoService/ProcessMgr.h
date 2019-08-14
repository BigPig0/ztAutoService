#pragma once

#include <windows.h>
#include <vector>
#include <string>
using namespace std;

struct ProcessInfo
{
    DWORD   lPID;             //< 进程ID
    time_t  nStartTime;       //< 程序启动时间

    TCHAR   szPath[MAX_PATH]; //< 执行文件路径
    TCHAR   szCmd[MAX_PATH];  //< 运行参数
    bool    bProtect;         //< 是否守护进程
    int     nReStartHour;     //< 每天重启的时间，单位时
    int     nRsStartDur;      //< 重启间隔
};

/**
 * 进程管理器，负责子进程的启动、保护、定时重启
 */
class CProcessMgr
{
public:
    CProcessMgr(void);
    ~CProcessMgr(void);

    /**
     * 启动pm
     */
    void Start();

    /**
     * 保护线程
     */
    bool ProtectRun();

    /**
     * 关闭所有子进程并退出
     */
    void Stop();

private:
    /**
     * 重启子进程
     */
    bool RunChild(ProcessInfo* pro);

    /**
     * 启动进程，并通过管道写入数据
     * @param nNum 同一个服务器上的进程任务编号
     * @param strDevInfo 该进程处理的设备的信息
     * @return true成功，false失败
     */
    bool CreateChildProcess(PTCHAR szPath, PTCHAR szCmd, DWORD& lPID);

    /**
     * 查找一个进程
     * @param lPID 进程ID
     */
    bool Find(DWORD lPID);

    /**
     * 结束一个进程
     * @param lPID 进程ID
     */
    bool Kill(DWORD lPID);

    vector<ProcessInfo*>    m_vecProcess;       //< 子进程信息,单线程不需要锁

    int  m_running; //正在执行的线程数
};