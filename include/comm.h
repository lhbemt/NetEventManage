//
// Created by li on 9/5/17.
//

#ifndef NETSERVEREVENT_COMM_H
#define NETSERVEREVENT_COMM_H

// event type define
enum class EVENT_TYPE
{
    EVENT_SOCKET, // io
    EVENT_SIG,    // signal
    EVENT_TIMER,  // timer
    EVENT_DB      // database
};

// network package define
struct Net_MessageHead
{
    int nLen; // length of package
};

#endif //NETSERVEREVENT_COMM_H
