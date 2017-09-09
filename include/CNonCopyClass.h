//
// Created by li on 9/9/17.
//

#ifndef NETSERVEREVENT_CNONCOPYCLASS_H
#define NETSERVEREVENT_CNONCOPYCLASS_H

class CNonCopyClass
{
public:
    CNonCopyClass() = default;
    CNonCopyClass(const CNonCopyClass& lhs) = delete;
    CNonCopyClass(CNonCopyClass&& rhs) = delete;
    CNonCopyClass&operator = (const CNonCopyClass& lhs) = delete;
    CNonCopyClass&operator = (CNonCopyClass&& rhs) = delete;
};



#endif //NETSERVEREVENT_CNONCOPYCLASS_H
