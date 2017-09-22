//
// Created by li on 9/5/17.
//

#ifndef NETSERVEREVENT_CSERVERMANAGE_H
#define NETSERVEREVENT_CSERVERMANAGE_H

#include "comm.h"
#include "CNonCopyClass.h"
#include "cclientmanage.h"
#include "csiganlmanage.h"
#include "ctimermanage.h"
#include "CThreadPool.h"

const char* strIp = "127.0.0.1";
const int   nPort = 8080;
const int   MAX_EPOLL_ENVS = 100000;

struct epoll_event;
class CServerManage : public CNonCopyClass
{
public:
    CServerManage();
    ~CServerManage();

    bool Start();

private:
    bool Init(); // init state
    bool InitListenSock();
    bool AttachToEpoll(int fd); // use et mode
    bool DettachEpoll(int fd);
    void DoAccept(struct epoll_event* env);
    void DoSignal(struct epoll_event* env);
    void DoTimer(struct epoll_event* env);
    void DoClientSockt(struct epoll_event* env);

private:
    CTimerManage        timerManage;
    CSignalManage       signalManage;
    CClientManage       clientManage;
    CThreadPool*        threadpool;

    int                 m_socklistenfd; // listen socket
    int                 m_epollfd; // epoll
    std::thread         m_epollThread; // epoll thread
    std::atomic<bool>   m_bRun; // stop
};


#endif //NETSERVEREVENT_CSERVERMANAGE_H
