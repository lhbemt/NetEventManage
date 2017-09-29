//
// Created by li on 9/5/17.
//

#include "../include/cservermanage.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <functional>

const char* strIp = "127.0.0.1";
const int   nPort = 8080;
const int   MAX_EPOLL_ENVS = 100000;

CServerManage::CServerManage()
{

}

CServerManage::~CServerManage()
{

}

void* ThreadsFuncServer(void* arg)
{
    CServerManage* pServer = (CServerManage*)arg;
    if (pServer)
    {
        int nRet = -1;
        epoll_event envs[MAX_EPOLL_ENVS];
        memset(envs, 0, sizeof(epoll_event) * MAX_EPOLL_ENVS);

        while(1)
        {
            nRet = epoll_wait(pServer->m_epollfd, envs, MAX_EPOLL_ENVS, -1);
            if (nRet < 0 && errno != EINTR) // EINTR wait again
            {
                return nullptr;
            }
            else if (nRet < 0 && errno == EINTR) // wait again
                continue;
            else
            {
                for (int i = 0; i < nRet; ++i) // ready fd
                {
                    if (envs[i].data.fd == pServer->m_socklistenfd) // accept
                        pServer->DoAccept(&envs[i]);
                    else if (envs[i].data.fd == pServer->m_pipefd[0]) // stop
                        return nullptr;
                    else if (envs[i].data.fd == pServer->signalManage.Getfd()) // signal
                        pServer->DoSignal(&envs[i]);
                    else if (envs[i].data.fd == pServer->timerManage.Getfd()) // timer
                        pServer->DoTimer(&envs[i]);
                    else // client socket
                        pServer->DoClientSockt(&envs[i]);
                }
            }
        }
    }
    return nullptr;
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
    int nRet = -1;
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
    // create epoll
    m_epollfd = epoll_create(5);
    if (m_epollfd < 0)
    {
        return false;
    }

    //create pipe to stop
    int nReturn = -1;
    nReturn = pipe(m_pipefd);
    if (nReturn != 0)
    {
        close(m_epollfd);
        return false;
    }
    fcntl(m_pipefd[0], F_SETFL, fcntl(m_pipefd[0], F_GETFL, 0) | O_NONBLOCK);
    bRet = AttachToEpoll(m_pipefd[0]);
    if (!bRet)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        return false;
    }

    bRet = signalManage.Init();
    if (!bRet)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        return false;
    }
    //attach to epoll
    bRet = AttachToEpoll(signalManage.Getfd());
    if (!bRet)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        signalManage.Stop();
        return false;
    }

    bRet = timerManage.Init();
    if (!bRet)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        signalManage.Stop();
        return false;
    }
    //attach to epoll
    bRet = AttachToEpoll(timerManage.Getfd());
    if (!bRet)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        signalManage.Stop();
        timerManage.Stop();
        return false;
    }

    threadpool = new(std::nothrow) CThreadPool(4);
    if (threadpool == nullptr)
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
        timerManage.Stop();
        signalManage.Stop();
        return false;
    }
    threadpool->Start();

    if (!InitListenSock())
    {
        close(m_epollfd);
        close(m_pipefd[0]);
        close(m_pipefd[1]);
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
    // epoll thread
//    int nRet = -1;
//    epoll_event envs[MAX_EPOLL_ENVS];
//    memset(envs, 0, sizeof(epoll_event) * MAX_EPOLL_ENVS);
//    m_epollThread = std::move(std::thread([this, &nRet, &envs](){
//        while(1)
//        {
//            nRet = epoll_wait(m_epollfd, envs, MAX_EPOLL_ENVS, -1);
//            if (nRet < 0)
//                return;
//            for (int i = 0; i < nRet; ++i) // ready fd
//            {
//                if (envs[i].data.fd == m_socklistenfd) // accept
//                    DoAccept(&envs[i]);
//                else if (envs[i].data.fd == m_pipefd[0]) // stop
//                    return;
//                else if (envs[i].data.fd == signalManage.Getfd()) // signal
//                    DoSignal(&envs[i]);
//                else if (envs[i].data.fd == timerManage.Getfd()) // timer
//                    DoTimer(&envs[i]);
//                else // client socket
//                    DoClientSockt(&envs[i]);
//            }
//        }
//    }));
    int nRet = -1;
    nRet = pthread_create(&m_epollThread, NULL, ThreadsFuncServer, this);
    if (nRet != 0)
        return false;
	return true;
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
                write(m_pipefd[1], "2", 1); // ternimate thread
                return;
            }
        }
        else
        {
            fcntl(nRet, F_SETFL, fcntl(nRet, F_GETFL, 0) | O_NONBLOCK);
            AttachToEpoll(nRet);
            clientManage.AddToManage(nRet);
        }
    }
}

void CServerManage::DoSignal(epoll_event *env) // siganl
{
    int nRet = -1;
    int signo = 0;
    while(1)
    {
        nRet = read(env->data.fd, &signo, sizeof(int));
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            else
            {
                write(m_pipefd[1], "2", 1);
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
        nRet = read(env->data.fd, &timerid, sizeof(int));
        if (nRet == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            else
            {
				write(m_pipefd[1], "2", 1);
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
            task.func = arg.callback;
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
            task.func = std::bind(&CTcpSocket::SendData, client, nullptr, 0);
            task.arg = NULL;
            threadpool->AddTask(std::move(task));
        }
    }
    return;
}

bool CServerManage::Stop()
{
    threadpool->Stop();
    delete threadpool;
    clientManage.CloseAllSocket();
    signalManage.Stop();
    timerManage.Stop();
    write(m_pipefd[1], "2", 1);
    //m_epollThread.join();
    pthread_join(m_epollThread, NULL);
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    close(m_epollfd);
    return true;
}

void CServerManage::RegisterEvent(EVENT_TYPE type, EventBase env, int milliseconds)
{
    switch(type)
    {
    case EVENT_TYPE::EVENT_SIG:
        signalManage.RegisterSiganl(env.fd, env.callBack, env.arg);
        break;
    case EVENT_TYPE::EVENT_TIMER:
        timerManage.SetTimer(env.fd, env.callBack, env.arg, milliseconds);
        break;
    case EVENT_TYPE::EVENT_SOCKET: // have done, no need
        break;
    }
}

void CServerManage::UnRegisterEvent(EVENT_TYPE type, EventBase env)
{
    switch (type) {
    case EVENT_TYPE::EVENT_SIG:
        signalManage.UnRegisterSiganl(env.fd);
        break;
    case EVENT_TYPE::EVENT_TIMER:
        timerManage.KillTimer(env.fd);
        break;
    case EVENT_TYPE::EVENT_SOCKET: // close socket
        clientManage.CloseSocket(env.fd);
        break;
    default:
        break;
    }
}
