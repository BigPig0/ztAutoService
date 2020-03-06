#pragma once
#include "util_def.h"
#include <windows.h>

/**
 * �̳߳��еĹ����߳�
 */
class Runnable
{
public:
    virtual ~Runnable() {}
    virtual void run() = 0;
};

/**
 * IOCPģ���е�I/O�߳�
 */
class IoRunnable
{
public:
    virtual ~IoRunnable() {}
    virtual void run(OVERLAPPED *o, uint32_t ioResult, uint64_t bytes, PTP_IO io) = 0;
};
