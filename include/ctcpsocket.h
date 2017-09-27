#ifndef CTCPSOCKET_H
#define CTCPSOCKET_H
#include <string.h>
#include <unistd.h>
#include "../include/comm.h"
#include "../include/lockmanage.h"

const int BUFF_SIZE = 1024 * 1024; // 1M

class CTcpSocket
{
public:
    CTcpSocket() : m_sockfd(0), m_nRecvLen(0), m_nSendLen(0)
    {
        memset(m_szSendBuff, 0, sizeof(m_szSendBuff));
        memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
    }

    void Init(int nSockfd)
    {
        m_sockfd = nSockfd;
        m_nRecvLen = 0;
        m_nSendLen = 0;
        memset(m_szSendBuff, 0, sizeof(m_szSendBuff));
        memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
    }

    void CloseSocket()
    {
        close(m_sockfd);
        m_nRecvLen = 0;
        m_nSendLen = 0;
        memset(m_szSendBuff, 0, sizeof(m_szSendBuff));
        memset(m_szRecvBuff, 0, sizeof(m_szRecvBuff));
    }

    void SendData(void* pData, int nLen);
    void RecvData();


private:
    int m_sockfd;

    char m_szRecvBuff[BUFF_SIZE];
    CMutex m_lRecvLock;
    int  m_nRecvLen;

    char m_szSendBuff[BUFF_SIZE];
    CMutex m_lSendLock;
    int  m_nSendLen;
};

#endif // CTCPSOCKET_H
