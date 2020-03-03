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

typedef enum{
    U_ENTER_GP_NAME,
    U_ENTER_NAME,
    U_PV_CHAT,
    U_SHOW_OPTIONS,
    U_WAITING,
    U_GP_CHAT,
    U_SEC_CHAT

} MyState;

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

void parse_request(Request* request, char *msg){
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
    strcpy(request->info, info);
    request->state = char_to_state(state);
}

char buffer_out[BUFSIZE];
char buffer_in[BUFSIZE];
char my_name[NAME_LEN];
int  my_port;
char sec_chat_name[NAME_LEN];
int  sec_chat_port;
char pv_chat_name[NAME_LEN];
char gp_chat_name[NAME_LEN];
int  gp_chat_port;
int  fd_master_server;
MyState myState;

void send_request(Request*, int);

void send_response(Response* response, int fd_client){
        memset(buffer_out, 0, sizeof(buffer_out));
        strcpy(buffer_out, state_to_char(response->state));
        strcat(buffer_out, "&");
        strcat(buffer_out, response->info);
        if(send(fd_client, buffer_out, strlen(buffer_out), 0) != strlen(buffer_out))
        { perror("# ERROR in sending response");}
        memset(buffer_out, 0, sizeof(buffer_out));
        return;
}

void run_sec_chat_client(){
    print(itoa(sec_chat_port, 10)); print("\n");
    int fd_client = create_tcp_socketFD();
    struct sockaddr_in address_server = create_address(sec_chat_port);
    int address_size = sizeof(address_server);
    print("trying connect to "); print(itoa(sec_chat_port, 10)); print("\n");
    if(connect(fd_client, (struct sockaddr *) &address_server, address_size)<0){
        perror("# ERROR Couldn't Connect to the Server");
        exit(EXIT_FAILURE);
    }
    print("connected to "); print(itoa(sec_chat_port, 10)); print("\n");
    print("$$$$$$$$$$$$$$$ CHAT WITH ");print(sec_chat_name);
    print(" $$$$$$$$$$$$$$$$$$\n");

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
            else{   // -------------------------------------- SERVER RESPONSES -----------------------
                buffer_in[read_size] = '\0';
                Response response;
                parse_response(buffer_in, &response);
                
                print(sec_chat_name); print(" : "); print(response.info); print("\n");

                if(response.state == _S_SEC_END){
                    Request request_done;
                    request_done.state = _C_SEC_END;
                    strcpy(request_done.info, my_name);
                    send_request(&request_done, fd_master_server);
                    break;
                }
            }
        }

        if(FD_ISSET(0, &fds_set)){ // ------------------------------- STDIN TO SERVER -------------------
            memset(buffer_in, '\0', sizeof(buffer_in));
            read(0, buffer_in, BUFSIZE);
            if(buffer_in[strlen(buffer_in)-1] == '\n')
                buffer_in[strlen(buffer_in)-1] = '\0';

            int bye = strcmp(buffer_in, "<exit>");
            
            char final[BUFSIZ];
            memset(final, 0, sizeof(final));
            strcpy(final, (bye == 0) ?  "BYE, I'M DONE." : buffer_in );

            Request request;
            request.state = (bye == 0) ? _C_SEC_END :  _C_SEC_SEND;
            strcpy(request.info, final);

            if(myState == U_SEC_CHAT){
                send_request(&request, fd_client);
                if(bye == 0){
                    Request request_done;
                    request_done.state = _C_SEC_END;
                    strcpy(request_done.info, my_name);
                    send_request(&request_done, fd_master_server);
                    break;
                }
            }

        }
    }

    close(fd_client);
}

void run_sec_chat_server(char* name_client){
    int fd_server                     = create_tcp_socketFD();
    struct sockaddr_in address_server = create_address(my_port);
    int address_len = sizeof(address_server);
    int opts = 1;
    if(setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opts, sizeof(opts)) < 0 ){
        perror("# ERROR in set socket opt");
        exit(EXIT_FAILURE);
    }
    bind_address_to_socket(&fd_server, &address_server);
    listen_to_connection(&fd_server, 10);
    
    Request request;
    request.state = _C_SEC_READY;
    strcpy(request.info, name_client);
    strcat(request.info, "&");
    strcat(request.info, itoa(my_port, 10));
    send_request(&request, fd_master_server);

    print("\n Entered to secret chat and waiting for client named : ");
    print(name_client); print("\n");

    int fd_client;
    struct sockaddr_in address_client;
    if((fd_client = accept(fd_server, (struct sockaddr*) &address_client, 
                            (socklen_t *) (&address_len))) < 0){
                perror("# ERROR in accepting client stuff");
                exit(EXIT_FAILURE);
    }
    
    print(name_client); print(" is connected !\n");

    print("$$$$$$$$$$$$$$$ CHAT WITH ");print(name_client);
    print(" $$$$$$$$$$$$$$$$$$\n");

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
                // client is gone !
            }
            else{   // -------------------------------------- SERVER RESPONSES -----------------------
                buffer_in[read_size] = '\0';
                // Response response;
                // parse_response(buffer_in, &response);

                Request request;
                parse_request(&request, buffer_in);
                print(name_client); print(" : "); print(request.info); print("\n");
                if(request.state == _C_SEC_END){
                    Request request_done;
                    request_done.state = _C_SEC_END;
                    strcpy(request_done.info, my_name);
                    send_request(&request_done, fd_master_server);
                    break;
                }
                
            }
        }

        if(FD_ISSET(0, &fds_set)){ // ------------------------------- STDIN TO SERVER -------------------
            memset(buffer_in, '\0', sizeof(buffer_in));
            read(0, buffer_in, BUFSIZE);
            if(buffer_in[strlen(buffer_in)-1] == '\n')
                buffer_in[strlen(buffer_in)-1] = '\0';

            int bye = strcmp(buffer_in, "<exit>");
            
            char final[BUFSIZ];
            memset(final, 0, sizeof(final));
            strcpy(final, (bye == 0) ?  "BYE, I'M DONE." : buffer_in );

            Response response;
            response.state = (bye == 0) ? _S_SEC_END : _S_SEC_SEND;
            strcpy(response.info, final);

            if(myState == U_SEC_CHAT){
                send_response(&response, fd_client);
                if(bye == 0){
                    Request request_done;
                    request_done.state = _C_SEC_END;
                    strcpy(request_done.info, my_name);
                    send_request(&request_done, fd_master_server);
                    break;
                }
            }

        }
    }

    close(fd_server);
}

void run_gp_chat(int port){
    gp_chat_port  = port;
    int fd_gp = create_udp_socketFD();
    struct sockaddr_in address_bc = create_broadcast_address(port);
    struct sockaddr_in address    = create_address(port);
    set_broadcast_options(fd_gp);
    bind_address_to_socket(&fd_gp, &address_bc);

    int len_bc_addr = sizeof(address_bc);

    fd_set fds_set;
    int _max_fd = fd_gp;

    while(true){
        FD_ZERO(&fds_set);
        FD_SET(fd_gp, &fds_set);
        FD_SET(0, &fds_set);
        
        if(((select(_max_fd + 1, &fds_set, NULL, NULL, NULL)) < 0) && (errno!=EINTR))
        { perror("# ERROR in selecting");}

        if(FD_ISSET(fd_gp, &fds_set)){
            int read_size;
            if((read_size = recvfrom(fd_gp, buffer_in, sizeof(buffer_in), MSG_WAITALL,
                            (struct sockaddr*)&address, &len_bc_addr)) <= 0){ 
                print("\nDAMN SON!\n");
            }
            else{   // ----------------------------------SERVER RESPONSES -------------
                buffer_in[read_size] = '\0';

                char name[NAME_LEN];
                char message[INFO_LEN];
                parse_chat_info(buffer_in, message, name);
                if(strcmp(name, my_name) != 0){
                    print(name); print(" : "); print(message); print("\n");
                }
               
            }
        }

        if(FD_ISSET(0, &fds_set)){ // -------------------------- STDIN TO SERVER ------
            memset(buffer_in, '\0', sizeof(buffer_in));
            read(0, buffer_in, BUFSIZE);
            if(buffer_in[strlen(buffer_in)-1] == '\n')
                buffer_in[strlen(buffer_in)-1] = '\0';

            int bye = strcmp(buffer_in, "<exit>");

            char final[BUFSIZ];
            memset(final, 0, sizeof(final));
            strcpy(final, my_name);
            strcat(final, "&");
            strcat(final, (bye == 0) ?  "BYE, I'M DONE." : buffer_in );

            if(myState == U_GP_CHAT){
                if((sendto(fd_gp, final, strlen(final), 0, 
                        (struct sockaddr*)&address_bc, len_bc_addr) <= 0)){
                    perror("ERRRR");
                }
                if(bye == 0){
                    break;
                }
            }
        }
    }
    close(fd_gp);
}

void print_request(Request request){
    print("("); print(state_to_char(request.state));
    print("&"); print(request.info); print(")"); print("\n");
}

void display_menu(){
    print("\n~~~~~~~~~~~ OPTIONS ~~~~~~~~~~~~~\n");
    print(itoa(_C_W_GPS_NAME, 10)); print(". See   All groups\n");
    print(itoa(_C_W_ADD_GP,   10)); print(". Make  a group\n");
    print(itoa(_C_W_GP_CHAT,  10)); print(". Start a group   chat\n");
    print(itoa(_C_W_PV_CHAT,  10)); print(". Start a private chat\n");
    print(itoa(_C_W_SEC_CHAT, 10)); print(". Start a Secret  Chat\n");
    print(itoa(_C_W_EXIT,     10)); print(". Exit\n");
    print("Enter ur option: ");
}

void send_request(Request* request, int fd_client){
        memset(buffer_out, '\0', sizeof(buffer_out));
        strcpy(buffer_out, state_to_char(request->state));
        strcat(buffer_out, "&");
        strcat(buffer_out, request->info);
        if(send(fd_client, buffer_out, strlen(buffer_out), 0) != strlen(buffer_out))
        { perror("# ERROR in sending messgae");}
        memset(buffer_out, '\0', sizeof(buffer_out));
        return;
}

void do_response(Response* response){
    // print("server:("); print(state_to_char(response->state));print(")\n");
    if(response->state == _S_WHO_R_U){
        print("enter ur name: ");
        myState = U_ENTER_NAME;
    }
    else if(response->state == _S_GPS_NAMES){
        print("###### GROUPS NAMES ######");
        print(response->info);
        print("\n#####################");
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_WHAT_U_WANT){
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_PV_BUSY){
        print(response->info); print("\n");
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_PV_STARTED){
        print("\n");
        strcpy(pv_chat_name, response->info);
        print("$$$$$$$$$$$$$$$ CHAT WITH ");print(response->info);
        print(" $$$$$$$$$$$$$$$$$$\n");
        myState = U_PV_CHAT;
    }
    else if(response->state == _S_PV_TAKE){
        char message[INFO_LEN];
        char to[NAME_LEN];
        parse_chat_info(response->info, message, to);
        print(pv_chat_name); print(":"); print(message);print("\n>");
        myState = U_PV_CHAT;
    }
    else if(response->state == _S_PV_END){
        print("\nChat is over!\n");
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_GP_PORT){
        print("$$$$$$$$$$$$$$$ CHAT WITH ");print(response->info);
        print(" $$$$$$$$$$$$$$$$$$\n");
        myState = U_GP_CHAT;
        run_gp_chat(atoi(response->info));
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state  == _S_GP_404){
        print("There is no group with name : ");
        print(gp_chat_name);
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_SEC_BUSY){
        print(response->info); print("\n");
        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_SEC_RUN){
        myState = U_SEC_CHAT;
        run_sec_chat_server(response->info);

        // TODO:sending to server that chat is over!

        myState = U_SHOW_OPTIONS;
        display_menu();
    }
    else if(response->state == _S_SEC_STARTED){
        myState = U_SEC_CHAT;
        sec_chat_port = atoi(response->info);
        run_sec_chat_client();

        // TODO:sending to server that chat is over

        myState = U_SHOW_OPTIONS;
        display_menu();
    }
}

void run_client(int server_port){
    
    int fd_client = create_tcp_socketFD();
    struct sockaddr_in address_server = create_address(server_port);
    int address_size = sizeof(address_server);
    if(connect(fd_client, (struct sockaddr *) &address_server, address_size)<0){
        perror("# ERROR Couldn't Connect to the Server");
        exit(EXIT_FAILURE);
    }
    fd_master_server = fd_client;
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
            else{   // -------------------------------------- SERVER RESPONSES -----------------------
                buffer_in[read_size] = '\0';
                Response response;
                parse_response(buffer_in, &response);
                do_response(&response);
            }
        }

        if(FD_ISSET(0, &fds_set)){ // ------------------------------- STDIN TO SERVER -------------------
            memset(buffer_in, '\0', sizeof(buffer_in));
            read(0, buffer_in, BUFSIZE);
            if(buffer_in[strlen(buffer_in)-1] == '\n')
                buffer_in[strlen(buffer_in)-1] = '\0';
            
            if(myState == U_ENTER_NAME){
                memset(my_name, 0, sizeof(my_name));
                strcpy(my_name, buffer_in);
                Request request;
                request.state = _C_MY_NAME_IS;
                strcpy(request.info, buffer_in);
                memset(buffer_in, '\0', sizeof(buffer_in));
                send_request(&request, fd_client);
            }

            else if(myState == U_PV_CHAT){
                if(strcmp(buffer_in, "<exit>") == 0){
                    Request request;
                    request.state = _C_PV_END;
                    strcpy(request.info, pv_chat_name);
                    print_request(request);
                    memset(buffer_in, '\0', sizeof(buffer_in));
                    send_request(&request, fd_client);
                    myState = U_SHOW_OPTIONS;
                    display_menu();
                }else{
                    Request request;
                    request.state = _C_PV_SEND;
                    strcpy(request.info, pv_chat_name);
                    strcat(request.info, "&");
                    strcat(request.info, buffer_in);
                    memset(buffer_in, '\0', sizeof(buffer_in));
                    send_request(&request, fd_client);
                    print(">");
                }
            }

            else if(myState == U_SHOW_OPTIONS){
                
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
                    myState = U_SHOW_OPTIONS;
                    display_menu();
                }

                if(atoi(buffer_in) == _C_W_GPS_NAME){
                    Request request;
                    request.state = _C_W_GPS_NAME;
                    send_request(&request, fd_client);
                    myState = U_WAITING;
                }
                
                if(atoi(buffer_in) == _C_W_PV_CHAT){
                    print("enter clients name: ");
                    memset(pv_chat_name, 0, sizeof(pv_chat_name));
                    read(0, pv_chat_name, NAME_LEN);
                    if(pv_chat_name[strlen(pv_chat_name)-1] == '\n')
                        pv_chat_name[strlen(pv_chat_name)-1] = '\0';
                    Request request;
                    strcpy(request.info, pv_chat_name);
                    request.state = _C_W_PV_CHAT;
                    send_request(&request, fd_client);
                    myState = U_WAITING;
                }

                if(atoi(buffer_in) == _C_W_GP_CHAT){
                    print("enter clients name: ");
                    memset(gp_chat_name, 0, sizeof(gp_chat_name));
                    read(0, gp_chat_name, NAME_LEN);
                    if(gp_chat_name[strlen(gp_chat_name)-1] == '\n')
                        gp_chat_name[strlen(gp_chat_name)-1] = '\0';
                    Request request;
                    strcpy(request.info, gp_chat_name);
                    request.state = _C_W_GP_CHAT;
                    send_request(&request, fd_client);
                    myState = U_WAITING;
                }

                if(atoi(buffer_in) == _C_W_SEC_CHAT){
                    print("enter clients name: ");
                    memset(sec_chat_name, 0, sizeof(sec_chat_name));
                    read(0, sec_chat_name, NAME_LEN);
                    if(sec_chat_name[strlen(sec_chat_name)-1] == '\n')
                        sec_chat_name[strlen(sec_chat_name)-1] = '\0';
                    Request request;
                    strcpy(request.info, sec_chat_name);
                    request.state = _C_W_SEC_CHAT;
                    send_request(&request, fd_client);
                    myState = U_WAITING;
                }
            }

        }

    }
    close(fd_client);
}


int main(int argc, char* argv[]){
    if(argc != 3){
        print(" :/ port please ! \n");
        exit(-1);
    }
    my_port = atoi(argv[2]);
    run_client(atoi(argv[1]));
    return 0;
}