#pragma once
#include "util.h"
#include <string>

class CProcessMgr
{
public:
    struct Options {
        std::string path;   //进程文件路径
        std::string args;   //执行参数
        bool        daemon; //是否守护进程
        int         delay;  //守护进程时，检测到进程关闭后，延迟多长时间再启动，单位秒
        std::string rstime; //重启的时间点，24小时制，eg:"15:30:00"
        int         rsdur;  //运行经过多久会重启，单位秒.
        std::string remark; //备注说明
        int         exedir; //工作目录 0:守护进程的工作目录 1:进程文件所在的目录 2:自定义
        std::string usrdir; //用户自定义的工作目录

        Options():daemon(true),delay(0),rsdur(0),exedir(0){};
    };

    struct Process : public Options {
        uint64_t    pid;    //进程ID
        time_t      startup;//启动时间点
        bool        running;
        bool        needclose;
        time_t      shutdown;//检测到退出的时间点
    };

    static CProcessMgr* Creat();

    virtual void Start() = 0;

    virtual void AddTask(Options opt) = 0;

    virtual void AddTasks(std::string path) = 0;

    virtual void Stop() = 0;

    virtual std::vector<Process> GetInfo() = 0;

    virtual ~CProcessMgr();
protected:
    CProcessMgr();
};