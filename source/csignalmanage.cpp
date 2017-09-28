#include "../include/csiganlmanage.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

int CSignalManage::m_pipefd[2];

void CSignalManage::RegisterSiganl(int signo, signalCallBack& callBack, void* arg)
{
    m_signalLock.Lock();
    signalFuncArg func;
    func.callBack = callBack;
    func.arg = arg;
    m_mapsignals.insert(std::make_pair(signo, func));
    signal(signo, SignalHandle); // use static
    m_signalLock.Unlock();
}

void CSignalManage::UnRegisterSiganl(int signo)
{
    m_signalLock.Lock();
    for (auto iter = m_mapsignals.begin(); iter != m_mapsignals.end(); ++iter)
    {
        if (iter->first == signo)
        {
            m_mapsignals.erase(iter);
            break;
        }
    }
    m_signalLock.Unlock();
}

bool CSignalManage::Init()
{
    int nRet = pipe(m_pipefd);
    if (nRet == -1)
        return false;
    fcntl(m_pipefd[0], F_SETFL, fcntl(m_pipefd[0], F_GETFL, 0) | O_NONBLOCK); // read should be nonblock
    return true;
}

void CSignalManage::SignalHandle(int signo)
{
    write(m_pipefd[1], &signo, sizeof(int)); // read fd[0], write fd[1]
}

void CSignalManage::Stop()
{
    m_signalLock.Lock();
    m_mapsignals.clear();
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    m_signalLock.Unlock();
}


















