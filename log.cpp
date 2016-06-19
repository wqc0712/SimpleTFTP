//
// Created by Tom Wang on 16/6/7.
//

#include "log.h"
#include <ctime>


void OpenLog() {
    if (InitFile) return;
    InitFile = true;
    string Name = ProgType+"log.txt";
    file=fopen(Name.c_str(),"a");
    return;
}

void PrintLog(char* data) {
    if (!InitFile) OpenLog();
    time_t Now;
    time(&Now);
    char* temp = ctime(&Now);
    char buff[100];
    int i;
    for (i = 0;temp[i] != '\n';i++) {
        buff[i] = temp[i];
    }
    buff[i] = '\0';
    fprintf(file,"%s %s",buff,data);
    return;
}

void CloseLog(){
    if (InitFile) {
        InitFile = false;
        fclose(file);
    }
}