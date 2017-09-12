#include "../include/ctcpsocket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

CTcpSocket::CTcpSocket()
{

}

int CTcpSocket::SendData(void *pData, int nLen)
{
    m_lSendLock.Lock();
    if (m_nSendLen + nLen + sizeof(Net_MessageHead) > BUFF_SIZE)
    {
        m_lSendLock.Unlock();
        return -2; // not enuogh buff
    }

    // copy to sendbuff
    memcpy(m_szSendBuff + m_nSendLen, &nLen, sizeof(nLen)); // len
    m_nSendLen += sizeof(nLen);
    memcpy(m_szSendBuff + m_nSendLen, pData, nLen);
    // deliver send
    int nRet = send(m_sockfd, m_szSendBuff, m_nSendLen, 0);
    if (nRet == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            m_lSendLock.Unlock();
            return nLen;
        }
        else // socket error
        {
            m_lSendLock.Unlock();
            return -1;
        }
    }

    m_lSendLock.Unlock();
    return nRet;
}

int CTcpSocket::RecvData() // use et mode
{
    m_lRecvLock.Lock();

    while(1)
    {
        int nRet = recv(m_sockfd, m_szRecvBuff + m_nRecvLen, BUFF_SIZE - m_nRecvLen, 0);
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                m_lRecvLock.Unlock();
                return -1;
            }
            else
            {
                m_lRecvLock.Unlock();
                return -2;
            }
        }
        else
        {
            m_nRecvLen += nRet;
            continue;
        }
    }

    // do something with recv buff

    m_lRecvLock.Unlock();
    return 0; // recv success





}
