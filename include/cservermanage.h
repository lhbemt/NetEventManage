//
// Created by li on 9/5/17.
//

#ifndef NETSERVEREVENT_CSERVERMANAGE_H
#define NETSERVEREVENT_CSERVERMANAGE_H

#include "comm.h"
#include "cnoncopyclass.h"
#include "cclientmanage.h"
#include "csiganlmanage.h"
#include "ctimermanage.h"
#include "cthreadpool.h"

struct epoll_event;

void* ThreadsFuncServer(void* arg);

class CServerManage : public CNonCopyClass
{
    friend void* ThreadsFuncServer(void* arg);
public:
    CServerManage();
    ~CServerManage();

    bool Start();
    bool Stop();
    // if register timer, need set milliseconds
    void RegisterEvent(EVENT_TYPE type, struct EventBase env, int milliseconds = 0);
    void UnRegisterEvent(EVENT_TYPE type, struct EventBase env);

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

    int                 m_pipefd[2]; // stop server
    int                 m_socklistenfd; // listen socket
    int                 m_epollfd; // epoll
    //std::thread         m_epollThread; // epoll thread
    pthread_t           m_epollThread; // epoll thread
};


#endif //NETSERVEREVENT_CSERVERMANAGE_H
