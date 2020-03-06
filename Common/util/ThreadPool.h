/**
 * ThreadPool��ص�api��Windows Vista/Windows Server 2008������ƽ̨���е�
 */

#pragma once
#include "util_public.h"
#include "Runnable.h"

class ThreadPool;
class ThreadRunnable : public Runnable
{
public:
    ThreadRunnable(){};
    virtual ~ThreadRunnable(){};
    virtual void run() = 0;

    PTP_WORK        m_pWork;
    ThreadPool*     m_pool;
};
class TimerRunnable : public Runnable
{
public:
    PTP_TIMER       m_pWork;
    ThreadPool*     m_pool;
};

////////////////////////////////////////////////////////////////////////////////
class UTIL_API ThreadPool : public LastError
{
public:
    explicit ThreadPool(int threadNum = 4);
    ~ThreadPool();

    /**
     * ��ʼ���̳߳ػ���
     */
    /*static*/ void init();
    /**
     * �����̳߳ػ���
     */
    /*static*/ void cleanup();
    /**
     * ��ȡϵͳCPU�ں�����
     * @return CPU�ں�����
     */
    static int cpuNum();

    /**
     * ����һ�������̷߳ŵ��̳߳��У�����ʱִ��runnable��run()
     * @param runnable[in] ҵ��ʵ������ʵ��������ҵ���߼��ģ��뺯�����洴���Ĺ����߳�ʵ������ͬһ��������
     * @param repeats[in] ҵ����ִ�д�����
     * @return �ɹ�true��ʧ��false
     */
    bool startWork(ThreadRunnable *runnable, int repeats = 1);

    /**
     * ����һ�������߳�
     * @param runnable[in] ҵ��ʵ��.
     * @return �ɹ�true��ʧ��false
     */
    bool closeWork(ThreadRunnable *runnable);

    /**
     * ע��һ���̳߳ض�ʱ����ʱ���ִ��runnable��run()���̳߳ض�ʱ����Ϊ�̳߳ط���ģ�������Ϊÿ�������̶߳�������ʱ����
     * @param runnable[in] ҵ��ʵ����
     * @param msDueTime[in] ��һ��������ʱ��(����) �����Ҫ����ִ����0
     * @param msPeriod[in] ִ�м��ʱ��(����)
     * @return �ɹ�true��ʧ��false
     */
    bool startTimer(TimerRunnable *runnable, int msDueTime, int msPeriod);

    /**
     * ����һ����ʱ��������ֻ��Ҫִ��һ�εĶ�ʱ���񣬿�����runnable��run����ʱ����
     */
    bool closeTimer(TimerRunnable *runnable);

    /**
     * ���ں˶��󴥷�ʱ����һ��������ִ��runnable��run()
     * @param runnable[in] ҵ��ʵ��.
     * @param event[in] �¼��ں˶���
     * @return �ɹ�true��ʧ��false
     */
    bool startWait(Runnable *runnable, HANDLE event);

    /**
     * ���첽I/O�������ʱ����һ��������ִ��runnable��run()
     * @param runnable[in] ҵ��ʵ��.
     * @param io[in] I/O���
     * @return �ɹ�true��ʧ��false
     */
    bool startIo(IoRunnable *runnable, HANDLE io);

    /**
     * �������̣߳����ر��̳߳�
     * @param wait[in] true�ȴ����ύ����δ����Ĺ�����ɣ�false��ǰ������ɣ�ȡ�����ύ����δִ�еĶ�����
     */
    void stop(bool wait = true);

private:
    static void doWork(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WORK work);
    static void expired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_TIMER timer);
    static void fired(PTP_CALLBACK_INSTANCE inst, void *p, PTP_WAIT wait, TP_WAIT_RESULT waitResult);
    static void ioCompletion(PTP_CALLBACK_INSTANCE inst, void *p, void *o, ULONG ioResult, ULONG_PTR bytes, PTP_IO io);

private:
    /*static*/ TP_CALLBACK_ENVIRON  m_env;              //< �ص�����
    PTP_POOL                    m_pool;             //< �̳߳�
    PTP_CLEANUP_GROUP           m_cleanupgroup;     //< ������
};

/**
 * Thread Local Storage
 * �̱߳��ش洢���������ܽ����ݺ�ִ�е��ض����߳���ϵ���������߳����ܷ���ȫ�ֱ�������������Ӱ�졣
 * ����ʵ�ֵ��Ƕ�̬TLS
 */
class UTIL_API Tls : public LastError
{
public:
    explicit Tls();
    ~Tls();

    bool get(void **pp);
    bool set(void *p);

private:
    DWORD m_tls;
};
