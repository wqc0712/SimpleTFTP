//
// Created by Tom Wang on 16/5/27.
//

#ifndef TELNET_EXCEPTIONSOCK_H
#define TELNET_EXCEPTIONSOCK_H

#include <string>

class ExceptionSock
{
public:
    ExceptionSock ( std::string s ) : m_s ( s ) {};
    ~ExceptionSock (){};

    std::string description() { return m_s; }

private:

    std::string m_s;

};

#endif //TELNET_EXCEPTIONSOCK_H
