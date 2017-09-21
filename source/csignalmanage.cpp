#include "../include/csiganlmanage.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

static int CSignalManage::m_pipefd[2];

void CSignalManage::RegisterSiganl(int signo, signalCallBack& callBack)
{
    m_mapsignals.insert(std::make_pair<int,signalCallBack>(signo, callBack));
    signal(signo, SignalHandle); // use static
}

void CSignalManage::UnRegisterSiganl(int signo)
{
    for (auto iter = m_mapsignals.begin(); iter != m_mapsignals.end(); ++iter)
    {
        if (iter->first == signo)
        {
            m_mapsignals.erase(iter);
            break;
        }
    }
}

bool CSignalManage::Init()
{
    int nRet = pipe(m_pipefd);
    if (nRet == -1)
        return false;
    fcntl(m_pipefd[0], F_SETFL, fcntl(m_pipefd[0], F_GETFL, 0) | O_NONBLOCK, 0); // read should be nonblock
    return true;
}

void CSignalManage::SignalHandle(int signo)
{
    write(m_pipefd[1], &signo, sizeof(int)); // read fd[0], write fd[1]
}


















