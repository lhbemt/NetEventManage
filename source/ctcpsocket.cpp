#include "../include/ctcpsocket.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

void CTcpSocket::SendData(void *pData, int nLen)
{
    m_lSendLock.Lock();
    if (m_nSendLen + nLen + sizeof(Net_MessageHead) > BUFF_SIZE)
    {
        m_lSendLock.Unlock();
        //return -2; // not enuogh buff
		return;
    }

    // copy to sendbuff
    //memcpy(m_szSendBuff + m_nSendLen, &nLen, sizeof(nLen)); // len
    //m_nSendLen += sizeof(nLen); // don't send length, just for test
    memcpy(m_szSendBuff + m_nSendLen, pData, nLen);
    m_nSendLen += nLen;
    // deliver send
    int nTotalSend = m_nSendLen;
    int nReadySend = 0;
    while(1)
    {
        int nRet = send(m_sockfd, m_szSendBuff + nReadySend, m_nSendLen - nReadySend, 0);
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // return immediately when buff full
            {
                // send failed beacause send buff was full, need send agin
                memcpy(m_szSendBuff, m_szSendBuff + nReadySend, nTotalSend - nReadySend);
                m_nSendLen = nTotalSend - nReadySend;
                m_lSendLock.Unlock();
                //return nLen;
				return;
            }
            else // socket error
            {
                m_lSendLock.Unlock();
                //return -1;
				return;
            }
        }
        else
        {
            nReadySend += nRet;
            if (nReadySend == nTotalSend)
                break; // send all
        }
    }

    memset(m_szSendBuff, 0, sizeof(m_szSendBuff));
    m_nSendLen = 0;
    m_lSendLock.Unlock();
    //return nLen; // send success
	return;
}

void CTcpSocket::RecvData() // use et mode
{
    m_lRecvLock.Lock();

    while(1)
    {
        int nRet = recv(m_sockfd, m_szRecvBuff + m_nRecvLen, BUFF_SIZE - m_nRecvLen, 0);
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //m_lRecvLock.Unlock();
                //return -2;
				break;
            }
            else
            {
                m_lRecvLock.Unlock();
                //return -1; // socket error
				return;
            }
        }
        else
        {
            m_nRecvLen += nRet;
            continue;
        }
    }

    // do something with recv buff
    // ...
    //
    //return 0; // recv success
	if (m_nRecvLen > 0) //echo client
	{
      SendData(m_szRecvBuff, m_nRecvLen);
	  memset(m_szRecvBuff, 0, BUFF_SIZE);
      m_nRecvLen = 0;
	}
	m_lRecvLock.Unlock();
	return;




}
