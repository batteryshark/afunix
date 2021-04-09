#include <stdio.h>
#include <stdarg.h>

#ifdef TARGET_OS_WINDOWS
#include <winsock2.h>
#define UNIX_PATH_MAX 108
typedef struct sockaddr_un{
     ADDRESS_FAMILY sun_family;     /* AF_UNIX */
     char sun_path[UNIX_PATH_MAX];  /* pathname */
} SOCKADDR_UN, *PSOCKADDR_UN;
#define close closesocket
#define EXPORTABLE __declspec(dllexport)
#else
#define EXPORTABLE __attribute__((visibility("default")))
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "client.h"

#ifdef TARGET_OS_WINDOWS
WSADATA WsaData = {0};
// Because we don't always have the luxury of MSVCRT.
#ifndef strlen
size_t __cdecl strlen(const char* str){
        const char* s;
        if (str == 0) return 0;
        for (s = str; *s; ++s);
        return s - str;
}
#endif
#ifndef strcpy
char *strcpy(char *dest, const char *src){
    size_t i;
    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}
#endif
#endif

int create_socket(const char* path){
    // Create a AF_UNIX stream server socket.
    int sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sd < 0) {return 0;}
    struct sockaddr_un serveraddr;
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path,path);
    if(connect(sd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_un)) < 0){
        close(sd);
        return 0;
    }
    return sd;
}

EXPORTABLE int SendMsg(const char* path, char* request){
    int sd = create_socket(path);
    if(!sd){return 0;}
    int rc = send(sd, request, strlen(request), 0);
    close(sd);
    if(rc == -1){return 0;}
    return 1;
}

EXPORTABLE int SendRecvMsg(const char* path, char* request, char* response, int response_max){
    int sd = create_socket(path);
    if(!sd){return 0;}
    if(send(sd, request, strlen(request), 0) == -1){
        close(sd);
        return 0;    
    }
    int bytes_read = recv(sd,response,response_max,0);
    close(sd);
    if(bytes_read == -1){return 0;}
    return 1;
}



#ifdef TARGET_OS_WINDOWS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:        
        WSAStartup(MAKEWORD(2, 2), &WsaData);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif