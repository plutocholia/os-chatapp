#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tools.h"

typedef struct{
    char  info[INFO_LEN];
    State state;
} Request;

typedef struct{
    char  info[INFO_LEN];
    State state;
} Response;


void parse_response(char *msg, Response* response){
    char state[STATE_LEN];
    char info[INFO_LEN];
    memset(info, '\0', sizeof(info));
    memset(state, '\0', sizeof(state));
    int i = 0, read_state = 1, j = 0;
    while( i < strlen(msg)){
        if(read_state){
            if(msg[i] == '&'){
               read_state = 0; 
               j = 0;
            }else{
                state[j] = msg[i];
                j += 1;
            }
        }
        else{
            info[j] = msg[i];
            j += 1;
        }
        i += 1;
    }
    strcpy(response->info, info);
    response->state = char_to_state(state);
}

char buffer_out[BUFSIZE];
char buffer_in[BUFSIZE];
State serverState;

void display_menu(){
    print("\n\n");
    print(itoa(_C_W_GPS_NAME,10));   print(". See All groups\n");
    print(itoa(_C_W_ADD_GP,  10));   print(". Make group\n");
    print(itoa(_C_W_GP_CHAT, 10));   print(". Enter to group\n");
    print(itoa(_C_W_PV_CHAT, 10));   print(". Enter to private chat\n");
    print(itoa(_C_W_EXIT,    10));   print(". Exit\n");
    print("Enter ur option: ");
}

void send_request(Request* request, int fd_client){
        memset(buffer_out, '\0', sizeof(buffer_out));
        strcpy(buffer_out, state_to_char(request->state));
        strcat(buffer_out, "&");
        strcat(buffer_out, request->info);
        // print("I've send "); print(buffer_out); print("\n");
        if(send(fd_client, buffer_out, strlen(buffer_out), 0) != strlen(buffer_out))
        { perror("# ERROR in sending messgae");}
        memset(buffer_out, '\0', sizeof(buffer_out));
        return;
}

void run_client(int server_port){
    
    int fd_client = create_tcp_socketFD();
    struct sockaddr_in address_server = create_address(server_port);
    int address_size = sizeof(address_server);
    if(connect(fd_client, (struct sockaddr *) &address_server, address_size)<0){
        perror("# ERROR Couldn't Connect to the Server");
        exit(EXIT_FAILURE);
    }
    
    fd_set fds_set;
    int _max_fd = fd_client;

    while(true){
        FD_ZERO(&fds_set);
        FD_SET(fd_client, &fds_set);
        FD_SET(0, &fds_set);
        
        if(((select(_max_fd + 1, &fds_set, NULL, NULL, NULL)) < 0) && (errno!=EINTR))
        { perror("# ERROR in selecting");}

        if(FD_ISSET(fd_client, &fds_set)){
            int read_size;
            if((read_size = read(fd_client, buffer_in, sizeof(buffer_in))) == 0){ 
                // server is gone !
            }
            else{
                buffer_in[read_size] = '\0';
                Response response;
                parse_response(buffer_in, &response);
                serverState = response.state;
                print("server:("); print(state_to_char(response.state)); 
                print("):"); print(response.info); print(" ");
                
                if(serverState == _S_PRINT){
                    serverState = _S_WHAT_U_WANT;
                    display_menu();
                }

                else if(serverState == _S_WHAT_U_WANT){
                    display_menu();
                }
            }
        }

        if(FD_ISSET(0, &fds_set)){
            memset(buffer_in, '\0', sizeof(buffer_in));
            read(0, buffer_in, BUFSIZE);
            if(buffer_in[strlen(buffer_in)-1] == '\n')
                buffer_in[strlen(buffer_in)-1] = '\0';
            
            if(serverState == _S_WHO_R_U){
                Request request;
                request.state = _C_MY_NAME_IS;
                strcpy(request.info, buffer_in);
                print(state_to_char(request.state)); print("()"); print(request.info); print("\n");
                memset(buffer_in, '\0', sizeof(buffer_in));
                send_request(&request, fd_client);
            }

            if(serverState == _S_WHAT_U_WANT){
                
                if(atoi(buffer_in) == _C_W_EXIT){
                    print("eybaba\n");
                    break;
                }
                
                if(atoi(buffer_in) == _C_W_ADD_GP){
                    print("enter a name for ur group: ");
                    char gpName[NAME_LEN];
                    memset(gpName, 0, sizeof(gpName));
                    read(0, gpName, NAME_LEN);
                    if(gpName[strlen(gpName)-1] == '\n')
                        gpName[strlen(gpName)-1] = '\0';
                    Request request;
                    strcpy(request.info, gpName);
                    request.state = _C_W_ADD_GP;
                    send_request(&request, fd_client);

                    serverState = _S_WHAT_U_WANT;
                    display_menu();
                }

                if(atoi(buffer_in) == _C_W_GPS_NAME){
                    Request request;
                    request.state = _C_W_GPS_NAME;
                    send_request(&request, fd_client);

                    serverState = _S_WHAT_U_WANT;
                }
                
            }
        }

    }
    close(fd_client);
}


int main(int argc, char* argv[]){
    if(argc != 2){
        print(" :/ port please ! \n");
        exit(-1);
    }
    run_client(atoi(argv[1]));
    return 0;
}