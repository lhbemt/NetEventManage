//
// Created by li on 9/6/17.
//

#ifndef NETSERVEREVENT_CTHREADPOOL_H
#define NETSERVEREVENT_CTHREADPOOL_H

#include "lockmanage.h"
#include "comm.h"
#include <iostream>
#include <list>
#include <functional>
#include <atomic>

const int MAX_THREADS = 10;

typedef std::function<void(void*)> taskFunc;
void* ThreadsFunc(void* arg);

struct Task
{
    taskFunc func;
    void*    arg;
};

class CThreadPool : CNonCopyClass
{
public:
    explicit CThreadPool(int nThreadNum) : m_nInitThreadNum(nThreadNum), m_bRun(false), m_nReady(0)
    {
        if (m_nInitThreadNum > 0 && m_nInitThreadNum <= MAX_THREADS)
            m_pThreads = new(std::nothrow) pthread_t[m_nInitThreadNum]; // new maybe faild
    }

    ~CThreadPool()
    {}

    bool Start(); // create threads
    bool AddTask(Task& task);
    bool AddTask(Task&& task);
    bool Stop(); // stop all

public:
    friend void* ThreadsFunc(void*);

private:
    int m_nInitThreadNum;
    std::list<Task> m_lstTasks;
    CMutex          m_lstLock;
    CConditionVar   m_condvar;
    pthread_t*      m_pThreads;
    std::atomic<bool> m_bRun;
    std::atomic<int> m_nReady; // have start thread
};


#endif //NETSERVEREVENT_CTHREADPOOL_H
