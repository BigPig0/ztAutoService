#pragma once

#include <windows.h>
#include <vector>
#include <string>
using namespace std;

struct ProcessInfo
{
    DWORD   lPID;             //< ����ID
    time_t  nStartTime;       //< ��������ʱ��

    TCHAR   szPath[MAX_PATH]; //< ִ���ļ�·��
    TCHAR   szCmd[MAX_PATH];  //< ���в���
    bool    bProtect;         //< �Ƿ��ػ�����
    int     nReStartHour;     //< ÿ��������ʱ�䣬��λʱ
    int     nRsStartDur;      //< �������
};

/**
 * ���̹������������ӽ��̵���������������ʱ����
 */
class CProcessMgr
{
public:
    CProcessMgr(void);
    ~CProcessMgr(void);

    /**
     * ����pm
     */
    void Start();

    /**
     * �����߳�
     */
    bool ProtectRun();

    /**
     * �ر������ӽ��̲��˳�
     */
    void Stop();

private:
    /**
     * �����ӽ���
     */
    bool RunChild(ProcessInfo* pro);

    /**
     * �������̣���ͨ���ܵ�д������
     * @param nNum ͬһ���������ϵĽ���������
     * @param strDevInfo �ý��̴�����豸����Ϣ
     * @return true�ɹ���falseʧ��
     */
    bool CreateChildProcess(PTCHAR szPath, PTCHAR szCmd, DWORD& lPID);

    /**
     * ����һ������
     * @param lPID ����ID
     */
    bool Find(DWORD lPID);

    /**
     * ����һ������
     * @param lPID ����ID
     */
    bool Kill(DWORD lPID);

    vector<ProcessInfo*>    m_vecProcess;       //< �ӽ�����Ϣ,���̲߳���Ҫ��

    int  m_running; //����ִ�е��߳���
};