#ifndef CTIMERMANAGE_H
#define CTIMERMANAGE_H
#include "../include/CNonCopyClass.h"
#include "../include/lockmanage.h"
#include <iostream>
#include <map>
#include <functional>
#include <time.h>
#include <list>
#include <atomic>
#include <thread>

typedef std::function<void()> timerCallBack;

struct TimerDefine
{
    int nTimerID;
    int nTickCount; // time travel
    struct timeval endTime;
};

class CTimerManage : public CNonCopyClass
{

public:
    CTimerManage() : m_bRun(false)
    {}
    ~CTimerManage()
    {}

    bool SetTimer(int timerID, timerCallBack& callBack, int milliseconds);
    void KillTimer(int timerID);
    void Init();
    void Tick();
    void Stop();

private:
    std::map<int, timerCallBack> m_mapTimer;
    std::list<TimerDefine*>      m_lstTimer;
    CMutex                       m_timerLock;
    std::atomic<bool>            m_bRun;
    std::thread                  m_tickThread;
    int                          m_pipefd[2];
}

#endif // CTIMERMANAGE_H
