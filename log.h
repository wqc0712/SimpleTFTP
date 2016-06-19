//
// Created by Tom Wang on 16/6/7.
//

#ifndef TELNET_LOG_H
#define TELNET_LOG_H
#include <iostream>
#include <fstream>


using namespace std;

extern string ProgType;

static bool InitFile = false;
static FILE * file;

void OpenLog();
void PrintLog(char* data);
void CloseLog();

#endif //TELNET_LOG_H
