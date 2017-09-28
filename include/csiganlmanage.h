#ifndef CSIGANLMANAGE_H
#define CSIGANLMANAGE_H

#include "../include/cnoncopyclass.h"
#include "../include/lockmanage.h"
#include <iostream>
#include <map>
#include <functional>

typedef std::function<void(void*)> signalCallBack;

struct signalFuncArg
{
    signalCallBack callBack;
    void*          arg;
};

class CSignalManage : public CNonCopyClass
{
public:
    CSignalManage()
    {}
    ~CSignalManage()
    {}

    bool Init(); // init pipe
    void RegisterSiganl(int signo, signalCallBack& callBack, void* arg);
    void UnRegisterSiganl(int signo);
    void Stop();
    int  Getfd() // attach to epoll
    {
        return m_pipefd[0];
    }

    signalFuncArg& GetCallBack(int signo, bool& bGet)
    {
        m_signalLock.Lock();
        auto iter = m_mapsignals.find(signo);
        m_signalLock.Unlock();
        if (iter != m_mapsignals.end())
        {
            bGet = true;
            return iter->second;
        }
    }

private:
    static void SignalHandle(int);

private:
    std::map<int, signalFuncArg> m_mapsignals;
    static int  m_pipefd[2];
    CMutex      m_signalLock;
};

#endif // CSIGANLMANAGE_H
