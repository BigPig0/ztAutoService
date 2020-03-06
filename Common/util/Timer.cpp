#include "Timer.h"

uint32_t        HighResolutionTimer::m_nInitCount = false;
uint64_t        HighResolutionTimer::m_nFrequency = 0;
CriticalSection HighResolutionTimer::m_cs;

uint32_t        TimerQueue::m_nInitCount = false;
HANDLE          TimerQueue::m_timerQueue = NULL;
CriticalSection TimerQueue::m_cs;


void HighResolutionTimer::init()
{
    MutexLock lock(&m_cs);
    if (!m_nInitCount)
    {
        m_nInitCount = true;
        /* ʹ�ø߾��ȼ����� */
        LARGE_INTEGER freq;
        // ���ض�ʱ����Ƶ��
        if(!QueryPerformanceFrequency(&freq))
        {
            /* Ӳ����֧��,ֻ��ʹ�ú�����Ϊ������ */
            m_nFrequency = 0;
        }
        else
        {
            m_nFrequency = freq.QuadPart;
        }
    }
}

uint64_t HighResolutionTimer::now()
{
    /* ���ص�ǰʱ���Ӧ��counter*/
    LARGE_INTEGER counter;
    if(m_nFrequency == 0)
    {
        // ���شӲ���ϵͳ�����������ĺ�����
        return GetTickCount();
    }
    else
    {
        // ���ض�ʱ����ǰ����ֵ
        if(!QueryPerformanceCounter(&counter))
            return GetTickCount();

        return counter.QuadPart;
    }
}

uint64_t HighResolutionTimer::getCounters(uint64_t ms)
{
    /* ��msʱ�任��Ϊ�������� */
    if( 0 == m_nFrequency )
    {
        /* Ӳ����֧�ָ߾��ȼ���,����������counter */
        return ms;
    }
    else
    {
        return ms * m_nFrequency / 1000;
    }
}

uint64_t HighResolutionTimer::getMs(uint64_t counter)
{
    if( 0 == m_nFrequency )
    {
        /* Ӳ����֧�ָ߾��ȼ���,��ʱ counter ���� ����*/
        return counter;
    }
    else
    {
        return static_cast<__int64>((counter * 1.0 / m_nFrequency) * 1000);
    }
}

uint64_t HighResolutionTimer::diffPC(uint64_t *a, uint64_t *b)
{
    if( 0 == m_nFrequency )
    {
        /* Ӳ����֧�ָ߾��ȼ���,a��b ���� ����*/
        return b - a;
    }
    else
    {
        return static_cast<uint64_t>((b - a) * 1.0 / m_nFrequency * 1000);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool TimerQueue::init()
{
    MutexLock lock(&m_cs);
    if(0 == m_nInitCount)
    {
        m_timerQueue = CreateTimerQueue();
        if (!m_timerQueue)
            return false;
    }
    m_nInitCount++;
    return true;
}

void TimerQueue::cleanup()
{
    m_nInitCount--;

    if (0 == m_nInitCount && m_timerQueue)
        DeleteTimerQueueEx(m_timerQueue, NULL);
}

////////////////////////////////////////////////////////////////////////////////
Timer::Timer() : m_timer(NULL)
{}

Timer::~Timer()
{
    if (m_timer)
        stop(false);
}


bool Timer::start(Runnable *runnable, int milliseconds, bool immediately)
{
    if (!CreateTimerQueueTimer(&m_timer, TimerQueue::m_timerQueue, (WAITORTIMERCALLBACK)&Timer::expired
        , runnable, milliseconds, immediately ? 0 : milliseconds, WT_EXECUTEDEFAULT))
        GOTO(failed);
    return true;
failed:
    return false;
}

bool Timer::stop(bool wait)
{
    HANDLE event;
    if (wait)
        event = INVALID_HANDLE_VALUE;
    else
        event = NULL;

    if (!DeleteTimerQueueTimer(TimerQueue::m_timerQueue, m_timer, event))
        GOTO(failed);
    m_timer = NULL;
    return true;
failed:
    return false;
}

void Timer::expired(void *p, BOOLEAN fired)
{
    Runnable *runnable = (Runnable *)p;
    runnable->run();
}
