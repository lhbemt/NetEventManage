#ifndef CCLIENTMANAGE_H
#define CCLIENTMANAGE_H

#include "cnoncopyclass.h"
#include <iostream>
#include <map>
#include "ctcpsocket.h"
#include "MemoryPool.h"

class CClientManage : public CNonCopyClass
{
public:
    CTcpSocket* GetSocket(int sockfd)
    {
        auto iter = m_mapClient.find(sockfd);
        if (iter != m_mapClient.end())
            return iter->second;
        return nullptr;
    }

    void CloseSocket(int sockfd)
    {
        auto iter = m_mapClient.find(sockfd);
        if (iter != m_mapClient.end())
        {
            iter->second->CloseSocket();
            m_mapClient.erase(iter);
        }
    }

    void CloseAllSocket()
    {
        for (auto& iter : m_mapClient)
            iter.second->CloseSocket();
        m_mapClient.clear();
    }

    bool AddToManage(int sockfd)
    {
        CTcpSocket* psocket = m_tcppool.GetElement();
        if (psocket)
        {
            psocket->Init(sockfd); // init first
            m_mapClient.insert(std::make_pair(sockfd, psocket));
            return true;
        }
        else
            return false;
    }

private:
    std::map<int, CTcpSocket*> m_mapClient;
    CMemoryPool<CTcpSocket>    m_tcppool;
};


#endif // CCLIENTMANAGE_H
