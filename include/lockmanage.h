//
// Created by li on 9/5/17.
//

#ifndef NETEVENTMANAGE_LOCKMANAGE_H
#define NETEVENTMANAGE_LOCKMANAGE_H
#include <pthread.h>
#include "CNonCopyClass.h"

class CMutex : public CNonCopyClass
{
public:
    CMutex()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }
    ~CMutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    bool Lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool Unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;
};

class CConditionVar : public CNonCopyClass
{
public:
    CConditionVar()
    {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    ~CConditionVar()
    {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mutex);
    }

    void Wait()
    {
        pthread_mutex_lock(&m_mutex);
        pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
    }

    void Signal()
    {
        pthread_cond_signal(&m_cond);
    }

    void SiganlAll()
    {
        pthread_cond_broadcast(&m_cond);
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
};

#endif //NETEVENTMANAGE_LOCKMANAGE_H
