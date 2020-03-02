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
#include <arpa/inet.h>

// ----------------------------- DEFINES --------------------------------------
#define BUFSIZE         1024
#define INFO_LEN        1000
#define STATE_LEN       24
#define NAME_LEN        256
#define true            1
#define false           0


#define S_WHO_R_U       "S_WHO_R_U"
#define S_WHAT_U_WANT   "S_WHAT_U_WANT"
#define S_GPS_NAMES     "S_GPS_NAMES"
#define S_PV_BUSY       "S_PV_BUSY"
#define S_PV_STARTED    "S_PV_STARTED"
#define S_PV_TAKE       "S_PV_TAKE"
#define S_PV_END        "S_PV_END"
#define S_GP_PORT       "S_GP_PORT"
#define S_GP_404        "S_GP_404"


#define C_GP_SEND       "C_GP_SEND"
#define C_PV_SEND       "C_PV_SEND"
#define C_PV_END        "C_PV_END"
#define C_CONNECT       "C_CONNECT"
#define C_MY_NAME_IS    "C_MY_NAME_IS"
#define C_W_GPS_NAME    "C_W_GPS_NAME"
#define C_W_PV_CHAT     "C_W_PV_CHAT"
#define C_W_GP_CHAT     "C_W_GP_CHAT"
#define C_W_ADD_GP      "C_W_ADD_GP"
#define C_W_EXIT        "C_W_EXIT"

typedef enum{
    _S_WHO_R_U      ,
    _S_WHAT_U_WANT  ,
    _S_GPS_NAMES    ,
    _S_PV_BUSY      ,
    _S_PV_STARTED   ,
    _S_PV_TAKE      ,
    _S_PV_END       ,
    _S_GP_PORT      ,
    _S_GP_404       ,
    
    _C_GP_SEND      ,
    _C_PV_SEND      ,
    _C_PV_END       ,
    _C_CONNECT      ,
    _C_MY_NAME_IS   ,
    _C_W_GPS_NAME   ,
    _C_W_PV_CHAT    ,
    _C_W_GP_CHAT    ,
    _C_W_ADD_GP     ,
    _C_W_EXIT       
} State;

// ---------------------------- TOOLS ----------------------------------------
State char_to_state(char *st){


    if(strcmp(st, S_WHO_R_U) == 0)
        return _S_WHO_R_U;
    
    if(strcmp(st, S_GPS_NAMES) == 0)
        return _S_GPS_NAMES;

    if(strcmp(st, S_WHAT_U_WANT) == 0)
        return _S_WHAT_U_WANT;

    if(strcmp(st, S_PV_BUSY) == 0)
        return _S_PV_BUSY;

    if(strcmp(st, S_PV_STARTED) == 0)
        return _S_PV_STARTED;

    if(strcmp(st, S_PV_TAKE) == 0)
        return _S_PV_TAKE;

    if(strcmp(st, S_PV_END) == 0)
        return _S_PV_END;

    if(strcmp(st, S_GP_PORT) == 0)
        return _S_GP_PORT;

    if(strcmp(st, S_GP_404) == 0)
        return _S_GP_404;


    if(strcmp(st, C_GP_SEND) == 0)
        return _C_GP_SEND;

    if(strcmp(st, C_PV_SEND) == 0)
        return _C_PV_SEND;

    if(strcmp(st, C_PV_END) == 0)
        return _C_PV_END;

    if(strcmp(st, C_CONNECT) == 0)
        return _C_CONNECT;

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

    if(st == _S_WHO_R_U)
        return S_WHO_R_U;

    if(st == _S_GPS_NAMES)
        return S_GPS_NAMES;
        
    if(st == _S_WHAT_U_WANT)
        return S_WHAT_U_WANT;
    
    if(st == _S_PV_BUSY)
        return S_PV_BUSY;

    if(st == _S_PV_STARTED)
        return S_PV_STARTED;

    if(st == _S_PV_TAKE)
        return S_PV_TAKE;
    
     if(st == _S_PV_END)
        return S_PV_END;
        
    if(st == _S_GP_PORT)
        return S_GP_PORT;

    if(st == _S_GP_404)
        return S_GP_404;


    if(st == _C_GP_SEND)
        return C_GP_SEND;

    if(st == _C_PV_END)
        return C_PV_END;

    if(st == _C_PV_SEND)
        return C_PV_SEND;

    if(st == _C_CONNECT)
        return C_CONNECT;

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

int create_udp_socketFD(){
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("# ERROR with creating upd socket");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

struct sockaddr_in create_broadcast_address(int port){
    struct sockaddr_in address;
    memset(&address, '\0', sizeof(address));
    address.sin_family = AF_INET;
    // address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    // inet_pton(AF_INET, "255.255.255.255", &address.sin_addr);
    address.sin_addr.s_addr = inet_addr("127.255.255.255");
    address.sin_port = htons(port);
    return address;
}

void set_broadcast_options(int fd){
    int opt3 = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt3, sizeof(opt3)) < 0){
        perror("# ERROR on setting broadcast options");
        close(fd);
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0){
        perror("# ERROR on setting broadcast options");
        close(fd);
        exit(EXIT_FAILURE);
    }
    // int opt2 = 1;
    // if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt2, sizeof(opt2)) < 0){
    //     perror("# ERROR on setting broadcast options");
    //     close(fd);
    //     exit(EXIT_FAILURE);
    // }
  
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

void parse_chat_info(char* info, char* msg, char* to){
    char message[INFO_LEN];
    char too[NAME_LEN];
    memset(message, '\0', sizeof(message));
    memset(too, '\0', sizeof(too));
    int i = 0, read_state = 1, j = 0;
    while( i < strlen(info)){
        if(read_state){
            if(info[i] == '&'){
               read_state = 0; 
               j = 0;
            }else{
                too[j] = info[i];
                j += 1;
            }
        }
        else{
            message[j] = info[i];
            j += 1;
        }
        i += 1;
    }
    strcpy(msg, message);
    strcpy(to, too);
}

void parse_info(char* info, char* a1, char* a2, char* a3){
    char _a1[INFO_LEN];
    char _a2[INFO_LEN];
    char _a3[INFO_LEN];
    memset(_a1, '\0', sizeof(_a1));
    memset(_a2, '\0', sizeof(_a2));
    memset(_a3, '\0', sizeof(_a3));

    int i = 0, read_state = 1, j = 0;
    while(i < strlen(info)){
        if(info[i] == '&'){
            read_state += 1;
            j = 0;
        }
        else if(read_state == 1){
            _a1[j] = info[i];
            j += 1;
        }
        else if(read_state == 2){
            _a2[j] = info[i];
            j += 1;
        }
        else{
            _a3[j] = info[i];
            j += 1;
        }
        i += 1;
    }
    strcpy(a1, _a1);
    strcpy(a2, _a2);
    strcpy(a3, _a3);
}


#endif