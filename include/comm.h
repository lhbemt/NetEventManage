//
// Created by li on 9/5/17.
//

#ifndef NETSERVEREVENT_COMM_H
#define NETSERVEREVENT_COMM_H
#include <iostream>
#include <functional>

static const char* strIp = "127.0.0.1";
static const int   nPort = 8080;
static const int   MAX_EPOLL_ENVS = 100000;


// event type define
enum class EVENT_TYPE
{
    EVENT_SOCKET, // io
    EVENT_SIG,    // signal
    EVENT_TIMER,  // timer
    EVENT_DB      // database
};
// evnet callback
typedef std::function<void(void*)> EventCallBack;
struct EventBase
{
    int fd; //socket fd,or signo, or timeid
    EventCallBack callBack; // callback
    void* arg; // arg
};

// network package define
struct Net_MessageHead
{
    int nLen; // length of package
};

#endif //NETSERVEREVENT_COMM_H
