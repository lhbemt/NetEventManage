//
// Created by li on 9/5/17.
//

#ifndef NETEVENTMANAGE_LOCKMANAGE_H
#define NETEVENTMANAGE_LOCKMANAGE_H
#include <pthread.h>
#include "cnoncopyclass.h"

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

    bool Wait()
    {
        int nRet = -1;
        nRet = pthread_mutex_lock(&m_mutex);
        if (nRet != 0)
            return false;
        nRet = pthread_cond_wait(&m_cond, &m_mutex);
        if (nRet != 0)
            return false;
        nRet = pthread_mutex_unlock(&m_mutex);
        if (nRet != 0)
            return false;
        return true;
    }

    bool Signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool SiganlAll()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
};

#endif //NETEVENTMANAGE_LOCKMANAGE_H
