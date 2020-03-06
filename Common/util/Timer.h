/**
 * TimerQueue���API�Ǵ�ͳ�ľ�api Windows XP/Windows Server 2003����ƽ̨ʹ��
 */
#pragma once
#include "util_public.h"
#include "Runnable.h"
#include "Mutex.h"
#include <stdint.h>

/**
 * ������ʱ���ȡ����
 */
class UTIL_API HighResolutionTimer
{
public:
    /** ��δ��ʼ�����ʼ�� */
    static void init();
    /** ���ص�ǰʱ�̵ļ��� */
    static uint64_t now();
    /* �������ܼ�����ʱ��������λ���� */
    static uint64_t diffPC(uint64_t *a, uint64_t *b);
    /** ����ʱ��תΪ���� */
    static uint64_t getCounters(uint64_t ms);
    /** ����תΪ����ʱ�� */
    static uint64_t getMs(uint64_t counter);
private:
    static uint64_t m_nFrequency; //��ʱ����Ƶ��
    static uint32_t m_nInitCount;
    static CriticalSection m_cs;
};

class UTIL_API TimerQueue
{
    friend class Timer;
public:
    /**
     * ����ȫ�ֵĶ�ʱ������
     */
    static bool init();

    /**
     * ɾ��ȫ�ֶ�ʱ������
     */
    static void cleanup();

private:
    /** ȫ�ֶ�ʱ������ʵ����� */
    static HANDLE          m_timerQueue;
    static uint32_t        m_nInitCount;
    static CriticalSection m_cs;
};

/**
 * ʹ�ö�ʱ�����еĶ�ʱ��ʵ��
 */
class UTIL_API Timer : public LastError
{
public:
    explicit Timer();
    ~Timer();

    /**
     * ע��һ����ʱ����ʱ���ִ��runnable��run()��
     * @param runnable[in] ҵ��ʵ����
     * @param milliseconds[in] ��ʱ�����ʱ��
     * @param immediately[in] �Ƿ�����ִ��
     * @return �ɹ�true��ʧ��false
     */
    bool start(Runnable *runnable, int milliseconds, bool immediately = false);

    /**
     * ������ɾ����ʱ������
     */
    bool stop(bool wait = true);

private:
    static void expired(void *p, BOOLEAN fired);

    HANDLE m_timer;
};

////////////////////////////////////////////////////////////////////////////////
