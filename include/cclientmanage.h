#ifndef CCLIENTMANAGE_H
#define CCLIENTMANAGE_H

#include "CNonCopyClass.h"
#include <iostream>
#include <map>
#include "ctcpsocket.h"

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
            iter->second->CloseSocket();
        m_mapClient.clear();
    }

    void AddToManage(int sockfd, CTcpSocket* tcp)
    {
        m_mapClient.insert(std::make_pair<int, CTcpSocket*>(sockfd, tcp));
    }

private:
    std::map<int, CTcpSocket*> m_mapClient;
};


#endif // CCLIENTMANAGE_H
