#pragma once
#include "util_public.h"

/**
 * ���������������һ�������������������������������
 * ��װ������������ַ������������Ľӿ�
 */
class UTIL_API CNetStreamMaker
{
public:
    CNetStreamMaker();
    virtual ~CNetStreamMaker();

    char* get(){return m_pData;}
    uint32_t size(){return m_nCurrent;}
    void clear(){m_nCurrent = 0;}

    /**
     * �����������������
     * @param data[in] ��Ҫ��ӵ�����
     * @param size[in] ���ݴ�С
     * @return �ɹ�true��ʧ��false
     */
    bool append_data(char* data, uint32_t size );

    /**
     * ����������׷��һ���ַ���(��0��β)
     */
    void append_string(const char *str );

    /**
     * ����������׷������
     */
    void append_byte(uint8_t  val);
    void append_be16(uint16_t val);
    void append_be24(uint32_t val);
    void append_be32(uint32_t val);
    void append_be64(uint64_t val);
    void append_bytes(uint8_t val, uint32_t num);
    void append_double(double val);

    /**
     * �޸��������е�����
     * @param start[in] ��Ҫ�޸ĵ����ݵ���ʼλ��
     * @param val[in] �µ�ֵ
     */
    void rewrite_byte(uint32_t start, uint8_t  val);
    void rewrite_be16(uint32_t start, uint16_t val);
    void rewrite_be24(uint32_t start, uint32_t val);
    void rewrite_be32(uint32_t start, uint32_t val);
    void rewrite_be64(uint32_t start, uint64_t val);
    void rewrite_double(uint32_t start, double val);

private:
    uint64_t dbl2int( double value );

private:
    char*    m_pData;      //�ڴ��ַ
    uint32_t m_nCurrent;   //��ǰд�����ݵ�λ��
    uint32_t m_nMax;       //�ڴ��С
};
