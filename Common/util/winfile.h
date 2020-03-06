/* Copyright (C) 2011 ������
 *
 * ����һ����Դ������,���������ɵ��޸ĺͷ���.
 * ��ֹ������ҵ��;.
 *
 * ��ϵԭ����: querw@sina.com 
*/

#pragma once
#include "util_public.h"

/*
* ����ϵͳ����ʹ��C��׼�⺯�� fopen() ͬʱ�򿪵��ļ���������(һ����512, ���� _setmaxstdio() ����2048)
* ���� HTTP ��������˵��ԶԶ������,���� class WINFile ֱ�ӵ��� Windows File API ��ʵ�ֺ� C��׼�⺯��ͬ���Ĺ���.
* WINFile ͬʱ�򿪵��ļ������Դﵽ 16384,���ҿ���ͨ�� Windows ���������� --max-open-files=N ���޸�.
*
*/

class UTIL_API WINFile
{
private:
	HANDLE _h;

public:
	WINFile();
	~WINFile();

	/*
	* ��ģʽ
	*/
	static const int r = 0x01;
	static const int w = 0x02;
	static const int rw = 0x04;

	static bool exist(const TCHAR *fileName);
	static bool remove(const TCHAR *fileName);

	bool open(const TCHAR *fileName, unsigned int mode, bool tmp = false);
	bool close();
	bool trunc();
	bool isopen();
	bool eof();

	unsigned long read(void *buf, unsigned long len);
	unsigned long write(const void *buf, unsigned long len);
	__int64 seek(__int64 off, DWORD mode);
	__int64 tell();
	__int64 size();
};

