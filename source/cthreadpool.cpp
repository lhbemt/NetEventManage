//
// Created by li on 9/6/17.
//

#include "../include/cthreadpool.h"
#include <unistd.h>

void* ThreadsFunc(void* arg)
{
    CThreadPool* pool = static_cast<CThreadPool*>(arg);
    pool->m_nReady.fetch_add(1);
    while(pool->m_bRun)
    {
        bool bRet = pool->m_condvar.Wait();
        if (!bRet)
            std::cout << "wake up failed" << std::endl;
        while (true)
        {
            pool->m_lstLock.Lock();
            if (!pool->m_lstTasks.empty())
            {
                Task task = std::move(pool->m_lstTasks.front());
                pool->m_lstTasks.pop_front();
                pool->m_lstLock.Unlock();
                // do task
                task.func(task.arg);
            }
            else
            {
                pool->m_lstLock.Unlock();
                break;
            }
        }
    }
    return nullptr;
}

bool CThreadPool::Start()
{
    if (m_nInitThreadNum <= 0 || m_nInitThreadNum > MAX_THREADS)
        return false;
    if (!m_pThreads)
        return false;
    m_bRun = true;
    for (int i = 0; i < m_nInitThreadNum; ++i)
    {
        pthread_create(m_pThreads + i, NULL, ThreadsFunc, this);
    }
    while(m_nReady < m_nInitThreadNum) // wait all thread run
        ;
    return true;
}

bool CThreadPool::AddTask(Task& task)
{
    if (!m_bRun)
        return false;
    m_lstLock.Lock();
    m_lstTasks.emplace_back(std::move(task));
    m_lstLock.Unlock();
    bool bRet = m_condvar.Signal(); // wake up one thread
    if (!bRet)
        std::cout << "siganal failed" << std::endl;
    return true;
}

bool CThreadPool::AddTask(Task&& task)
{
    if (!m_bRun)
        return false;
    m_lstLock.Lock();
    m_lstTasks.emplace_back(task);
    m_lstLock.Unlock();
    bool bRet = m_condvar.Signal(); // wake up one thread
    if (!bRet)
        std::cout << "siganal failed" << std::endl;
    return true;
}

bool CThreadPool::Stop()
{
    if (!m_bRun)
        return false;
    m_bRun = false;
    m_condvar.SiganlAll(); // wake up all
    for (int i = 0; i < m_nInitThreadNum; ++i)
        pthread_join(*(m_pThreads + i), NULL);
    std::cout << "remain task: " << m_lstTasks.size() << std::endl;
    for (auto& task : m_lstTasks)
        if (task.arg)
            delete (char*)task.arg;
    m_lstTasks.clear();
    return true;
}
