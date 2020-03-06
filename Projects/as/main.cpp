/*!
 * �޲���ֱ������������ֱ�����У�������as.json��Ϊ����
 * һ����������(as.exe xxx), ����ֱ�����У�������xxx.json��Ϊ���á�xxx������-��ʼ
 * һ������-i����(as.exe -i), ���ᰲװһ������Ϊas�ķ��񣬲��������������������as.json��Ϊ����
 * ������������(as.exe -i xxx), ���ᰲװһ������Ϊxxx�ķ��񣬲��������������������xxx.json��Ϊ����
 * һ������-u����(as.exe -u), ����ֹͣ��ж��as����
 * ������������(as.exe -u xxx), ����ֹͣ��ж��xxx����
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
    if ( CTRL_C_EVENT == type           //�û�����Ctrl+C,�رճ���
        || CTRL_BREAK_EVENT == type     //�û�����CTRL+BREAK
        || CTRL_LOGOFF_EVENT == type    //�û��˳�ʱ(ע��)
        || CTRL_SHUTDOWN_EVENT == type  //��ϵͳ���ر�ʱ
        || CTRL_CLOSE_EVENT == type)    //����ͼ�رտ���̨����
    {
        Log::debug("stop all child process and exit");
        if(_pm) delete _pm;
        //return TRUE;
    }
    return FALSE;
}

/**
 * �������������������ʵ�ֶԷ���Ŀ��ƣ����ڷ����������ֹͣ����������ʱ���������д˴�����
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
    //ע��������
    _hServiceStatus = RegisterServiceCtrlHandler(_szSvrName, ServiceStrl);
    if (_hServiceStatus == NULL) {
        Log::warning("Handler not installed");
        return;
    }

    //����״̬���������
    _status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    _status.dwWin32ExitCode = S_OK;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;
    _status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(_hServiceStatus, &_status);
    Log::debug("service is running now");

    //����ִ������
    _pm = CProcessMgr::Creat();
    char config[MAX_PATH] = {0};
    sprintf(config, "%s\\%s.json", _strDir.c_str(), _szSvrName);
    _pm->AddTasks(config);
    _pm->Start();
    while(_status.dwCurrentState == SERVICE_RUNNING)
        Sleep(1000);
    _pm->Stop();

    //ִ�н���
    _status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(_hServiceStatus, &_status);
    Log::debug("Service stopped");
}

bool IsInstalled() {
    bool bResult = FALSE;

    //�򿪷�����ƹ�����
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM != NULL) {
        //�򿪷���
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

    //�򿪷�����ƹ�����
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL) {
        Log::error("Couldn't open service manager");
        return false;
    }

    // ��ȡ���������·��
    char szFilePath[MAX_PATH];
    ::GetModuleFileNameA(NULL, szFilePath, MAX_PATH);

    string strCmd = "\"";
    strCmd += szFilePath;
    strCmd += "\"";
    strCmd += " -s ";
    strCmd += _szSvrName;

    Log::debug("install service name:%s path:%s", _szSvrName, strCmd.c_str());

    //��������
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

    //����ϵͳ����
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

    //ɾ������
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
        /** ���ÿ���̨��Ϣ�ص� */
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);

        if(argc == 2)
            _szSvrName = argv[1];
        else
            _szSvrName = "as";

        /** ������־�ļ� */
        char path[MAX_PATH];
        sprintf(path, "%s\\log\\%s.txt", _strDir.c_str(), _szSvrName);
        Log::open(Log::Print::both, Log::Level::debug, path);
        Log::debug("version: %s %s", __DATE__, __TIME__);

        //���������ػ�
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
            /** ������־�ļ� */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sInstall.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            Install();
        } else if(!strcmp(argv[1], "-u")) {
            /** ������־�ļ� */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sUninstall.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            Uninstall();
        } else if(!strcmp(argv[1], "-s")) {
            _dwThreadID = ::GetCurrentThreadId();

            /** ������־�ļ� */
            char path[MAX_PATH];
            sprintf(path, "%s\\log\\%sService.txt", _strDir.c_str(), _szSvrName);
            Log::open(Log::Print::both, Log::Level::debug, path);
            Log::debug("version: %s %s", __DATE__, __TIME__);

            // ��������
            SERVICE_TABLE_ENTRY st[] = {
                { _szSvrName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
                { NULL, NULL }
            };
            if (!::StartServiceCtrlDispatcher(st)) {
                Log::error("Register Service Main Function Error!");
            }
        } else {
            //�޷������Ĳ���
            Log::open(Log::Print::both, Log::Level::debug, ".\\log\\errarg.txt");
            Log::debug("version: %s %s", __DATE__, __TIME__);
            for(int i=1; i<argc; i++){
                Log::error("arg%d : %s", i, argv[i]);
            }
        }
    } else {
        //�޷������Ĳ���
        Log::open(Log::Print::both, Log::Level::debug, ".\\log\\errarg.txt");
        Log::debug("version: %s %s", __DATE__, __TIME__);
        for(int i=1; i<argc; i++){
            Log::error("arg%d : %s", i, argv[i]);
        }
    }
    return 0;
}