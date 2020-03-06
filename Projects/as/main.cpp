/*!
 * 无参数直接启动，将会直接运行，并加载as.json作为配置
 * 一个参数启动(as.exe xxx), 将会直接运行，并加载xxx.json作为配置。xxx不能以-开始
 * 一个参数-i启动(as.exe -i), 将会安装一个名称为as的服务，并启动。服务启动后加载as.json作为配置
 * 两个参数启动(as.exe -i xxx), 将会安装一个名称为xxx的服务，并启动。服务启动后加载xxx.json作为配置
 * 一个参数-u启动(as.exe -u), 将会停止并卸载as服务
 * 两个参数启动(as.exe -u xxx), 将会停止并卸载xxx服务
 */

#include "utilc_api.h"
#include "stdio.h"
#include "pm.h"
#include "Log.h"

CProcessMgr* _pm = NULL;
SERVICE_STATUS_HANDLE _hServiceStatus = NULL;
SERVICE_STATUS _status = {
    SERVICE_WIN32_OWN_PROCESS,
    SERVICE_STOPPED,
    SERVICE_ACCEPT_STOP,
    0
};
DWORD _dwThreadID = 0;
char *_szSvrName = NULL;
string _strDir;

BOOL CtrlCHandler(DWORD type)
{
    if ( CTRL_C_EVENT == type           //用户按下Ctrl+C,关闭程序。
        || CTRL_BREAK_EVENT == type     //用户按下CTRL+BREAK
        || CTRL_LOGOFF_EVENT == type    //用户退出时(注销)
        || CTRL_SHUTDOWN_EVENT == type  //当系统被关闭时
        || CTRL_CLOSE_EVENT == type)    //当试图关闭控制台程序
    {
        Log::debug("stop all child process and exit");
        if(_pm) delete _pm;
        //return TRUE;
    }
    return FALSE;
}

/**
 * 服务控制主函数，这里实现对服务的控制，当在服务管理器上停止或其它操作时，将会运行此处代码
 */
void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		_status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(_hServiceStatus, &_status);
		PostThreadMessage(_dwThreadID, WM_CLOSE, 0, 0);
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
		Log::error("Bad service request");
	}
}

void WINAPI ServiceMain()
{
    //注册服务控制
    _hServiceStatus = RegisterServiceCtrlHandler(_szSvrName, ServiceStrl);
    if (_hServiceStatus == NULL) {
        Log::warning("Handler not installed");
        return;
    }

    //服务状态变成已启动
    _status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    _status.dwWin32ExitCode = S_OK;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;
    _status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(_hServiceStatus, &_status);
    Log::debug("service is running now");

    //服务执行内容
    _pm = CProcessMgr::Creat();
    char config[MAX_PATH] = {0};
    sprintf(config, "%s\\%s.json", _strDir.c_str(), _szSvrName);
    _pm->AddTasks(config);
    _pm->Start();
    while(_status.dwCurrentState == SERVICE_RUNNING)
        Sleep(1000);
    _pm->Stop();

    //执行结束
    _status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(_hServiceStatus, &_status);
    Log::debug("Service stopped");
}

bool IsInstalled() {
    bool bResult = FALSE;

    //打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM != NULL) {
        //打开服务
        SC_HANDLE hService = ::OpenServiceA(hSCM, _szSvrName, SERVICE_QUERY_CONFIG);
        if (hService != NULL) {
            bResult = true;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

bool Install() {
    if (IsInstalled())
        return true;

    //打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        Log::error("Couldn't open service manager");
        return false;
    }

    // 获取程序自身的路径
    char szFilePath[MAX_PATH];
    ::GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

    string strCmd = "\"";
    strCmd += szFilePath;
    strCmd += "\"";
    strCmd += " -s ";
    strCmd += _szSvrName;

    Log::debug("install service name:%s path:%s", _szSvrName, strCmd.c_str());

    //创建服务
    SC_HANDLE hService = ::CreateServiceA(
        hSCM, _szSvrName, _szSvrName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
        strCmd.c_str(), NULL, NULL, "", NULL, NULL);

    if (hService == NULL) {
        ::CloseServiceHandle(hSCM);
        Log::error("Couldn't create service");
        return false;
    }

    //启动系统服务
    if (StartService(hService, 0, NULL) == false) {
        ::CloseServiceHandle(hService);
        ::CloseServiceHandle(hSCM);
        return false;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return true;
}

bool Uninstall() {
    if (!IsInstalled())
        return true;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        Log::error("Couldn't open service manager");
        return false;
    }

    SC_HANDLE hService = ::OpenService(hSCM, _szSvrName, SERVICE_STOP | DELETE);
    if (hService == NULL) {
        ::CloseServiceHandle(hSCM);
        Log::error("Couldn't open service");
        return false;
    }

    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    //删除服务
    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return true;

    Log::error("Service could not be deleted");
    return false;
}

int main(int argc, char* argv[])
{
    char szFilePath[MAX_PATH];
    ::GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
    _strDir = szFilePath;
    _strDir = _strDir.substr(0, _strDir.rfind("\\"));

    if(argc == 1 || (argc == 2 && argv[1][0] != '-')) {
        /** 设置控制台消息回调 */
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);

        if(argc == 2)
            _szSvrName = argv[1];
        else
            _szSvrName = "as";

        /** 创建日志文件 */
        char path[MAX_PATH];
        sprintf(path, "%s\\log\\%s.txt", _strDir.c_str(), _szSvrName);
        Log::open(Log::Print::both, Log::Level::debug, path);
        Log::debug("version: %s %s", __DATE__, __TIME__);

        //启动进程守护
        _pm = CProcessMgr::Creat();
        char config[MAX_PATH] = {0};
        sprintf(config, "%s\\%s.json", _strDir.c_str(), _szSvrName);
        _pm->AddTasks(config);
        _pm->Start();

        sleep(INFINITE);
    } else if((argc == 2 || argc == 3)) {
        if(argc == 3)
            _szSvrName = argv[2];
        else
            _szSvrName = "as";

        if(!strcmp(argv[1], "-i")) {
            /** 创建日志文件 */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sInstall.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            Install();
        } else if(!strcmp(argv[1], "-u")) {
            /** 创建日志文件 */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sUninstall.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            Uninstall();
        } else if(!strcmp(argv[1], "-s")) {
            _dwThreadID = ::GetCurrentThreadId();

            /** 创建日志文件 */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sService.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            // 启动服务
            SERVICE_TABLE_ENTRY st[] = {
                { _szSvrName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
                { NULL, NULL }
            };
            if (!::StartServiceCtrlDispatcher(st)) {
                Log::error("Register Service Main Function Error!");
            }
        } else {
            //无法解析的参数
            Log::open(Log::Print::both, Log::Level::debug, ".\\log\\errarg.txt");
            Log::debug("version: %s %s", __DATE__, __TIME__);
            for(int i=1; i<argc; i++){
                Log::error("arg%d : %s", i, argv[i]);
            }
        }
    } else {
        //无法解析的参数
        Log::open(Log::Print::both, Log::Level::debug, ".\\log\\errarg.txt");
        Log::debug("version: %s %s", __DATE__, __TIME__);
        for(int i=1; i<argc; i++){
            Log::error("arg%d : %s", i, argv[i]);
        }
    }
    return 0;
}