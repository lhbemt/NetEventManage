//
// Created by li on 9/5/17.
//

#include "../include/CServerManage.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

CServerManage::CServerManage() : m_bRun(false)
{

}

CServerManage::~CServerManage()
{

}

bool CServerManage::AttachToEpoll(int fd)
{
    epoll_event env;
    memset(&env, 0, sizeof(env));
    env.data.fd = fd;
    env.events |= EPOLLIN;
    env.events |= EPOLLOUT; // socket send, when send buff is not full again
    env.events |= EPOLLET; // et mode
    int nRet = -1;
    nRet = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &env);
    if (nRet < 0)
        return false;
    return true;
}

bool CServerManage::DettachEpoll(int fd)
{
    in nRet = -1;
    nRet = epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, NULL);
    if (nRet < 0)
        return false;
    return true;
}

bool CServerManage::InitListenSock()
{
    m_socklistenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socklistenfd < 0)
        return false;
    // set nonblock
    fcntl(m_socklistenfd, F_SETFL, fcntl(m_socklistenfd, F_GETFL, 0) | O_NONBLOCK);
    // set reuse addr
    int nReuse = 1;
    int nRet = -1;
    nRet = setsockopt(m_socklistenfd, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(int));
    if (nRet < 0)
    {
        close(m_socklistenfd);
        return false;
    }
    // bind
    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(nPort);
    nRet = inet_pton(AF_INET, strIp, &serveraddr.sin_addr);
    if (nRet != 1)
    {
        close(m_socklistenfd);
        return false;
    }
    nRet = bind(m_socklistenfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
    if (nRet < 0)
    {
        close(m_socklistenfd);
        return false;
    }
    //listen
    nRet = listen(m_socklistenfd, SOMAXCONN);
    if (nRet < 0)
    {
        close(m_socklistenfd);
        return false;
    }
    // add to epoll
    return AttachToEpoll(m_socklistenfd);
}

bool CServerManage::Init()
{
    bool bRet = false;
    bRet = signalManage.Init();
    if (!bRet)
        return false;
    bRet = timerManage.Init();
    if (!bRet)
    {
        signalManage.Stop();
        return false;
    }

    threadpool = new(std::nothrow) CThreadPool(4);
    if (threadpool == nullptr)
    {
        timerManage.Stop();
        signalManage.Stop();
        return false;
    }
    threadpool->Start();

    // create epoll
    m_epollfd = epoll_create(5);
    if (m_epollfd < 0)
    {
        timerManage.Stop();
        signalManage.Stop();
        threadpool->Stop();
        return false;
    }

    if (!InitListenSock())
    {
        timerManage.Stop();
        signalManage.Stop();
        threadpool->Stop();
        return false;
    }
    return true;
}

bool CServerManage::Start()
{
    if (!Init())
        return false;
    m_bRun = true;
    // epoll thread
    int nRet = -1;
    epoll_event envs[MAX_EPOLL_ENVS];
    memset(envs, 0, sizeof(epoll_event) * MAX_EPOLL_ENVS);
    m_epollThread = std::move(std::thread([this, &nRet, &envs](){
        while(m_bRun)
        {
            nRet = epoll_wait(m_epollfd, envs, MAX_EPOLL_ENVS, -1);
            if (nRet < 0)
                return;
            for (int i = 0; i < nRet; ++i) // ready fd
            {
                if (envs[i].data.fd == m_socklistenfd) // accept
                    DoAccept(&envs[i]);
                else if (envs[i].data.fd == signalManage.Getfd()) // signal
                    DoSignal(&envs[i]);
                else if (envs[i].data.fd == timerManage.Getfd()) // timer
                    DoTimer(&envs[i]);
                else // client socket
                    DoClientSockt(&envs[i]);
            }
        }
    }));
}

void CServerManage::DoAccept(epoll_event *env)
{
    int nRet = -1;
    while(1)
    {
        nRet = accept(env->data.fd, NULL, NULL);
        if (nRet < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return;
            else // wrong with accept
            {
                DettachEpoll(env->data.fd);
                m_bRun = false; // ternimate thread
                return;
            }
        }
        else
        {
            fcntl(nRet, F_SETFL, fcntl(nRet, F_GETFL, 0) | O_NONBLOCK);
            AttachToEpoll(nRet);
        }
    }
}

void CServerManage::DoSignal(epoll_event *env) // siganl
{
    int nRet = -1;
    int signo = 0;
    while(1)
    {
        nRet = recv(env->data.fd, &signo, sizeof(int), 0);
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            else
            {
                m_bRun = false;
                DettachEpoll(env->data.fd);
                return;
            }
        }
        // add to threadpool
        bool bGet = false;
        signalFuncArg arg = signalManage.GetCallBack(signo, bGet);
        if (bGet)
        {
            Task task;
            task.func = arg.callBack;
            task.arg = arg.arg;
            threadpool->AddTask(std::move(task));
        }
    }
}

void CServerManage::DoTimer(epoll_event *env)
{
    int nRet = -1;
    int timerid = 0;
    while(1)
    {
        nRet = recv(env->data.fd, &timerid, sizeof(int), 0);
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            else
            {
                m_bRun = false;
                DettachEpoll(env->data.fd);
                return;
            }
        }
        // add to threadpool
        bool bGet = false;
        timerFuncArg arg = timerManage.GetCallBack(timerid, bGet);
        if (bGet)
        {
            Task task;
            task.func = arg.callBack;
            task.arg = arg.arg;
            threadpool->AddTask(std::move(task));
        }
    }
}

void CServerManage::DoClientSockt(epoll_event *env)
{
    if (env->events & EPOLLIN) // read
    {
        CTcpSocket* client = clientManage.GetSocket(env->data.fd);
        if (client)
        {
            Task task;
            task.func = std::bind(&CTcpSocket::RecvData, client);
            task.arg = NULL;
            threadpool->AddTask(std::move(task));
        }
    }
    else if (env->events & EPOLLOUT) // send
    {
        CTcpSocket* client = clientManage.GetSocket(env->data.fd);
        if (client)
        {
            Task task;
            task.func = std::bind(&CTcpSocket::SendData, client, NULL, 0);
            task.arg = NULL;
            threadpool->AddTask(std::move(task));
        }
    }
    return;
}






















