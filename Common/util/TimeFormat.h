#pragma once
#include <windows.h>
#include <time.h>

class CTimeFormat
{
public:

    /**
     * ��һ����ʽΪyyyyMMddhhmmss��ʽ���ַ���ת��ʱ������
     */
    static time_t scanTime(const char buf[32]);

    /**
     * ��ӡ��ǰʱ��
     * @param fmt ����ĸ�ʽ(���������룬�����Ӵ�����Ϻ�����)
     * @param buf ������ݵ��ڴ�
     */
    static char *printNow(const char *fmt, char buf[32]);

    /**
     * �����ǰʱ���ַ���
     * @param fmt ����ĸ�ʽ
     */
    static string printNow(const char *fmt);

    /**
     * ��ӡָ��ʱ��
     * @param time ָ����ʱ��
     * @param fmt ����ĸ�ʽ
     * @param buf ������ݵ��ڴ�
     */
    static char *printTime(time_t *time, const char *fmt, char buf[32]);

    static string printTime(time_t time, const char *fmt);

private:

};

inline
time_t CTimeFormat::scanTime(const char buf[32])
{
    struct tm tm;

    sscanf_s(buf, "%4d%2d%2d%2d%2d%2d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec);

    tm.tm_year -= 1900;
    tm.tm_mon --;
    tm.tm_isdst = -1;

    return mktime(&tm);
}

inline
char *CTimeFormat::printNow(const char *fmt, char buf[32])
{
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    struct tm tm;
    tm.tm_year      = systime.wYear - 1900;
    tm.tm_mon       = systime.wMonth - 1;
    tm.tm_mday      = systime.wDay;
    tm.tm_hour      = systime.wHour;
    tm.tm_min       = systime.wMinute;
    tm.tm_sec       = systime.wSecond;
    tm.tm_isdst     = -1;

    char stamp[32];
    strftime(stamp, 32, fmt, &tm);
    sprintf_s(buf, 32, "%s.%03d", stamp, systime.wMilliseconds);
    return buf;
}

inline
string CTimeFormat::printNow(const char *fmt) {
    time_t now = time(NULL);
    struct tm tm;
    localtime_s(&tm, &now);

    char buf[32];
    strftime(buf, 32, fmt, &tm);
    return buf;
}

inline
char *CTimeFormat::printTime(time_t *time, const char *fmt, char buf[32])
{
    struct tm tm;
    localtime_s(&tm, time);

    char stamp[32];
    strftime(stamp, 32, fmt, &tm);
    sprintf_s(buf, 32, "%s", stamp);
    return buf;
}

inline
string CTimeFormat::printTime(time_t time, const char *fmt) {
    struct tm tm;
    localtime_s(&tm, &time);

    char buf[32];
    strftime(buf, 32, fmt, &tm);
    return buf;
}