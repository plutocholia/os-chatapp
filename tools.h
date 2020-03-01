#ifndef TOOLS_H
#define TOOLS_H

#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// ----------------------------- DEFINES --------------------------------------
#define BUFSIZE         1024
#define INFO_LEN        1000
#define STATE_LEN       24
#define NAME_LEN        256
#define true            1
#define false           0

#define C_CONNECT         "C_CONNECT"
#define S_WHO_R_U         "S_WHO_R_U"
#define S_WHAT_U_WANT     "S_WHAT_U_WANT"
#define S_PRINT           "S_PRINT"
#define C_MY_NAME_IS      "C_MY_NAME_IS"

#define C_W_GPS_NAME      "C_W_GPS_NAME"
#define C_W_PV_CHAT       "C_W_PV_CHAT"
#define C_W_GP_CHAT       "C_W_GP_CHAT"
#define C_W_ADD_GP        "C_W_ADD_GP"
#define C_W_EXIT          "C_W_EXIT"

typedef enum{
    _C_CONNECT      ,
    _S_WHO_R_U      ,
    _S_WHAT_U_WANT  ,
    _S_PRINT        ,
    _C_MY_NAME_IS   ,
    _C_W_GPS_NAME   ,
    _C_W_PV_CHAT    ,
    _C_W_GP_CHAT    ,
    _C_W_ADD_GP     ,
    _C_W_EXIT       
} State;

// ---------------------------- TOOLS ----------------------------------------
State char_to_state(char *st){

    if(strcmp(st, C_CONNECT) == 0)
        return _C_CONNECT;

    if(strcmp(st, S_WHO_R_U) == 0)
        return _S_WHO_R_U;
    
    if(strcmp(st, S_PRINT) == 0)
        return _S_PRINT;

    if(strcmp(st, S_WHAT_U_WANT) == 0)
        return _S_WHAT_U_WANT;

    if(strcmp(st, C_MY_NAME_IS) == 0)
        return _C_MY_NAME_IS;

    if(strcmp(st, C_W_GPS_NAME) == 0)
        return _C_W_GPS_NAME;

    if(strcmp(st, C_W_PV_CHAT) == 0)
        return _C_W_PV_CHAT;
    
    if(strcmp(st, C_W_GP_CHAT) == 0)
        return _C_W_GP_CHAT;

    if(strcmp(st, C_W_ADD_GP) == 0)
        return _C_W_ADD_GP;

    if(strcmp(st, C_W_EXIT) == 0)
        return _C_W_EXIT;

    return -1;
}

char* state_to_char(State st){

    if(st == _C_CONNECT)
        return C_CONNECT;

    if(st == _S_WHO_R_U)
        return S_WHO_R_U;

    if(st == _S_PRINT)
        return S_PRINT;
        
    if(st == _S_WHAT_U_WANT)
        return S_WHAT_U_WANT;

    if(st == _C_MY_NAME_IS)
        return C_MY_NAME_IS;

    if(st == _C_W_GPS_NAME)
        return C_W_GPS_NAME;
    
    if(st == _C_W_PV_CHAT)
        return C_W_PV_CHAT;

    if(st == _C_W_GP_CHAT)
        return C_W_GP_CHAT;
    
    if(st == _C_W_ADD_GP)
        return C_W_ADD_GP;

    if(st == _C_W_EXIT)
        return C_W_EXIT;

    return "(NULL)";
}



void print(char* buf) {
    write(1, buf, strlen(buf));
}

char* itoa(int val, int base){
    static char buf[32] = {0};
	
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

char* super_strcat(int n, ...){
    va_list valist;
    va_start(valist, n);

    int i, len = 0;
    for(i = 0; i < n; i++)
        len += strlen(va_arg(valist, char*));
    va_end(valist);

    char* main = (char*) malloc((len + 1)* sizeof(char));
    va_start(valist, n);
    for(i = 0; i < n; i++)
        strcat(main, va_arg(valist, char*));
    va_end(valist);

    return main;
}


int max(int a, int b){
    return (a >= b) ? a : b;
}

// -------------------------- SOCKET PART ----------------------------------


struct sockaddr_in create_address(int port){
    struct sockaddr_in address;
    memset(&address, '\0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    return address;
}

void bind_address_to_socket(int* fd, struct sockaddr_in* address){
    if((bind(*fd, (struct sockaddr *) address, sizeof(*address))) < 0){
        perror("# ERROR in binding socket with address");
        exit(EXIT_FAILURE);
    }
}

void listen_to_connection(int* fd, int backlog){
    if((listen(*fd, backlog)) < 0 ){
        perror("# ERROR in listening");
    }
}

int create_tcp_socketFD(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("# ERROR with creating tcp socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void handle_fdset(int *_max, fd_set *fds_set, int *fds, int len){
    int i;
    for(i = 0; i < len; i++){
        if(fds[i] > 0){
            FD_SET(fds[i], fds_set);
        }
         *_max = max(fds[i], *_max);
    }
}

#endif