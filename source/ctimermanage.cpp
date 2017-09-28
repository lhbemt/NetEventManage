#include "../include/ctimermanage.h"
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>

bool CTimerManage::SetTimer(int timerID, timerCallBack &callBack, void* arg, int milliseconds)
{
    if (!m_bRun)
        return false;

    m_timerLock.Lock();
    auto iter = m_mapTimer.find(timerID);
    if (iter != m_mapTimer.end()) // already set
    {
        m_timerLock.Unlock();
        return false;
    }
    struct timeval valNow;
    struct TimerDefine* endTime = new TimerDefine;
    gettimeofday(&valNow, NULL);
    if (valNow.tv_usec + milliseconds * 1000 > 1000000)
    {
        endTime->endTime.tv_sec = valNow.tv_sec + (valNow.tv_usec + milliseconds * 1000) / 1000000;
        endTime->endTime.tv_usec = (valNow.tv_usec + milliseconds * 1000) % 1000000;
    }
    else
    {
        endTime->endTime.tv_sec = valNow.tv_sec;
        endTime->endTime.tv_usec = valNow.tv_usec + milliseconds * 1000;
    }
    endTime->nTickCount = milliseconds;
    endTime->nTimerID = timerID;
    m_lstTimer.push_back(endTime);
    timerFuncArg func;
    func.callback = callBack;
    func.arg = arg;
    m_mapTimer.insert(std::make_pair(timerID, func));
    m_timerLock.Unlock();
	return true;
}

void CTimerManage::KillTimer(int timerID)
{
    if (!m_bRun)
        return;

    m_timerLock.Lock();
    auto iter = m_mapTimer.find(timerID);
    if (iter == m_mapTimer.end())
    {
        m_timerLock.Unlock();
        return;
    }
    m_mapTimer.erase(iter);

    for (auto iter = m_lstTimer.begin(); iter != m_lstTimer.end(); ++iter)
    {
        if ((*iter)->nTimerID == timerID)
        {
            delete *iter;
            m_lstTimer.erase(iter);
            break;
        }
    }

    m_timerLock.Unlock();
}

bool CTimerManage::Init()
{
    int nRet = pipe(m_pipefd);
    if (nRet == -1)
        return false;
    nRet = fcntl(m_pipefd[0], F_SETFL, fcntl(m_pipefd[0], F_GETFL, 0) | O_NONBLOCK); // read should be nonblock
    Tick();
    return true;
}

void CTimerManage::Tick()
{
    m_bRun = true;
    m_tickThread = std::move(std::thread([this](){
       while(m_bRun)
       {
           struct timeval tm;
           tm.tv_sec = 0;
           tm.tv_usec = 10000; // 10ms tick one
           select(1, NULL, NULL, NULL, &tm);
           m_timerLock.Lock();
           struct timeval now;
           gettimeofday(&now, NULL);
           for (auto& ltime : m_lstTimer)
           {
               if (ltime->endTime.tv_sec < now.tv_sec)
               {
                   write(m_pipefd[1], &ltime->nTimerID, sizeof(int));
                   if (now.tv_usec + ltime->nTickCount * 1000 > 1000000)
                   {
                       ltime->endTime.tv_sec = now.tv_sec + (now.tv_usec + ltime->nTickCount * 1000) / 1000000;
                       ltime->endTime.tv_usec = (now.tv_usec + ltime->nTickCount * 1000) % 1000000;
                   }
                   else
                   {
                       ltime->endTime.tv_sec = now.tv_sec;
                       ltime->endTime.tv_usec = now.tv_usec + ltime->nTickCount * 1000;
                   }
               }
               else if (ltime->endTime.tv_sec == now.tv_sec)
               {
                   if (ltime->endTime.tv_usec <= now.tv_usec)
                   {
                       write(m_pipefd[1], &ltime->nTimerID, sizeof(int));
                       if (now.tv_usec + ltime->nTickCount * 1000 > 1000000)
                       {
                           ltime->endTime.tv_sec = now.tv_sec + (now.tv_usec + ltime->nTickCount * 1000) / 1000000;
                           ltime->endTime.tv_usec = (now.tv_usec + ltime->nTickCount * 1000) % 1000000;
                       }
                       else
                       {
                           ltime->endTime.tv_sec = now.tv_sec;
                           ltime->endTime.tv_usec = now.tv_usec + ltime->nTickCount * 1000;
                       }
                   }
               }
           }
           m_timerLock.Unlock();
       }
    }));
}

void CTimerManage::Stop()
{
    m_bRun = false;
    m_tickThread.join(); // wait end

    m_timerLock.Lock();
    for (auto& ltime : m_lstTimer)
        delete ltime;

    m_lstTimer.clear();
    m_mapTimer.clear();
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    m_timerLock.Unlock();
}
