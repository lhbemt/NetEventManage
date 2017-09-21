#ifndef CSIGANLMANAGE_H
#define CSIGANLMANAGE_H

#include "../include/CNonCopyClass.h"
#include <iostream>
#include <map>
#include <functional>

typedef std::function<void()> signalCallBack;

class CSignalManage : public CNonCopyClass
{
public:
    CSignalManage()
    {}
    ~CSignalManage()
    {}

    bool Init(); // init pipe
    void RegisterSiganl(int signo, signalCallBack& callBack);
    void UnRegisterSiganl(int signo);

private:
    static void SignalHandle(int);

private:
    std::map<int, signalCallBack> m_mapsignals;
    static int  m_pipefd[2];
}

#endif // CSIGANLMANAGE_H
