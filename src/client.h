#pragma once 

int SendMsg(const char* path, char* request);
int SendRecvMsg(const char* path, char* request, char* response, int response_max);