#ifndef CTIMERMANAGE_H
#define CTIMERMANAGE_H
#include "../include/cnoncopyclass.h"
#include "../include/lockmanage.h"
#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <list>
#include <atomic>
#include <thread>

typedef std::function<void(void*)> timerCallBack;

struct TimerDefine
{
    int nTimerID;
    int nTickCount; // time travel
    struct timeval endTime;
};

struct timerFuncArg
{
    timerCallBack callback;
    void*         arg;
};

class CTimerManage : public CNonCopyClass
{

public:
    CTimerManage() : m_bRun(false)
    {}
    ~CTimerManage()
    {}

    bool SetTimer(int timerID, timerCallBack& callBack, void* arg, int milliseconds);
    void KillTimer(int timerID);
    bool Init();
    void Tick();
    void Stop();
    int  Getfd()
    {
        return m_pipefd[0];
    }

    timerFuncArg& GetCallBack(int timerid, bool& bGet)
    {
        m_timerLock.Lock();
        auto iter = m_mapTimer.find(timerid);
        m_timerLock.Unlock();
        if (iter != m_mapTimer.end())
        {
            bGet = true;
            return iter->second;
        }
    }

private:
    std::map<int, timerFuncArg> m_mapTimer;
    std::list<TimerDefine*>      m_lstTimer;
    CMutex                       m_timerLock;
    std::atomic<bool>            m_bRun;
    std::thread                  m_tickThread;
    int                          m_pipefd[2];
};

#endif // CTIMERMANAGE_H
