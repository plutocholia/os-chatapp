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

#define MAX_SERVER_CAPA     100
#define MAX_USERS           100
#define MAX_GROUP_LEN       30

typedef struct{
    int   port;
    char  name[NAME_LEN];
    int   online;
    int   busy;
} User;

typedef struct{
    User* users[MAX_USERS];
    char  name[NAME_LEN];
    int   online;
    int   port;
} Group;

typedef struct{
    User* user;
    char  info[INFO_LEN];
    State state;
} Request;

typedef struct{
    User* user;
    char  info[INFO_LEN];
    State state;
} Response;


User  users[MAX_USERS];
Group groups[MAX_GROUP_LEN];

char  buffer_out[BUFSIZE];
char  buffer_in[BUFSIZE];
int   user_count = 0;
int   gps_count = 0;

// ------------------------------ Users Stuff ---------------------

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

User* users_init(int port){
   users[user_count].port = port;
   users[user_count].online = true;
   users[user_count].busy = false;
   memset(users[user_count].name, '\0', sizeof(users[user_count].name));
   user_count += 1;
   return &users[user_count - 1];
}

int users_find_index(int port){
    int i;
    for(i = 0; i < MAX_USERS; i++)
        if(users[i].port == port)
            return i;
    return -1;
}

User* users_return(int port){
    return &(users[users_find_index(port)]);
}

void users_delete(User *user){
    // *user = {0};
    memset(user, 0, sizeof(User));
}


// -------------------------- GRAOUP Stuff ------------------------------

// Group* group_return(char* name){
//     int i;
//     for(i = 0; i < )
// }


void create_groups_file(){
    int fd;
    if((fd = open("Server/groups", O_CREAT | O_EXCL)) < 0){
        perror("# ERROR creating groups file");
    }else{
        perror("groups file is created");
    }
    close(fd);
}

void add_group(char *group_name){
    int fd;
    if((fd = open("Server/groups", O_APPEND | O_WRONLY)) < 0){
        perror("# ERROR opening group file");
        return;
    }
    write(fd, group_name, strlen(group_name));
    close(fd);
}


// --------------------------------------------------------------------------

void send_response(Response* response, int fd_client){
        memset(buffer_out, 0, sizeof(buffer_out));
        strcpy(buffer_out, state_to_char(response->state));
        strcat(buffer_out, "&");
        strcat(buffer_out, response->info);
        if(send(fd_client, buffer_out, strlen(buffer_out), 0) != strlen(buffer_out))
        { perror("# ERROR in sending gretting messgae");}
        memset(buffer_out, 0, sizeof(buffer_out));
        return;
}

void do_request(Request* request, int fd_client){
    if(request->state == _C_CONNECT){
        Response response;
        response.user = request->user;
        strcpy(response.info, "whats ur name?");
        response.state = _S_WHO_R_U;
        send_response(&response, fd_client);
        return;
    }
    if(request->state == _C_MY_NAME_IS){
        strcpy(request->user->name, request->info);
        Response response;
        response.state = _S_WHAT_U_WANT;
        strcpy(response.info, "what do u want?");
        response.user = request->user;
        print("* port "); print(itoa(request->user->port, 10)); print(" is ");
        print(request->user->name); print("\n");
        send_response(&response, fd_client);
    }
    if(request->state == _C_W_ADD_GP){
        Group new_gp;
        strcpy(new_gp.name, request->info);
        new_gp.online = false;
        groups[gps_count++] = new_gp;
        print("* "); print(request->info); print(" is added to gps"); print("\n");
    }

    if(request->state == _C_W_GPS_NAME){
        int i;
        Response response;
        response.state = _S_PRINT;
        for(i = 0; i < MAX_GROUP_LEN; i++){
            if(strlen(groups[i].name) > 0){
                strcat(response.info, "\n");
                strcat(response.info, groups[i].name);
            }
        }
        send_response(&response, fd_client);
    }
}
//------------------------------------ SERVER Stuff --------------------

void run_server(int server_port){
    create_groups_file();

    // initailization of listenning socket
    int fd_server                     = create_tcp_socketFD();
    struct sockaddr_in address_server = create_address(server_port);
    int address_len = sizeof(address_server);
    int opts = 1;
    if(setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opts, sizeof(opts)) < 0 ){
        perror("# ERROR in set socket opt");
        exit(EXIT_FAILURE);
    }
    bind_address_to_socket(&fd_server, &address_server);
    listen_to_connection(&fd_server, 10);
    print(super_strcat(3, "+ server is running on port ", itoa(server_port, 10), " ...\n"));
    
    // accepting conections
    fd_set fds_set;
    int fds[MAX_SERVER_CAPA];
    
    memset(&fds, 0, sizeof(fds));
    int selected, fd_client, _max, i;
    struct sockaddr_in address_client;

    while(true){
        FD_ZERO(&fds_set);
        FD_SET(fd_server, &fds_set);
        _max = fd_server;

        // initialization of fd_set
        handle_fdset(&_max, &fds_set, fds, MAX_SERVER_CAPA);

        // select part
        if(((selected = select(_max + 1, &fds_set, NULL, NULL, NULL)) < 0) && (errno!=EINTR))
        { perror("# ERROR in selecting");}


        // if somethign happens
        if(FD_ISSET(fd_server, &fds_set)){

            if((fd_client = accept(fd_server, (struct sockaddr*) &address_client, 
                            (socklen_t *) (&address_len))) < 0){
                perror("# ERROR in accepting client stuff");
                exit(EXIT_FAILURE);
            }
            print("+ new connection on port : "); 
            print(itoa(ntohs(address_client.sin_port), 10)); print("\n");

            User* user = users_init(ntohs(address_client.sin_port)); 
            Request request;
            request.user  = user;
            request.state = _C_CONNECT;
            do_request(&request, fd_client);

            // add client fd to the list
            for( i = 0; i < MAX_SERVER_CAPA; i++){
                if(fds[i] == 0){
                    fds[i] = fd_client;
                    // psrint("New User is Added "); print(itoa(i, 10)); print(" \n");
                    break;
                }
            }
        }

        // from other clients
        for(i = 0; i < MAX_SERVER_CAPA; i++){
            fd_client = fds[i];
            if(FD_ISSET(fd_client, &fds_set)){
                int sent_size;

                struct sockaddr_in temp;
                getpeername(fd_client, (struct sockaddr *) &temp, 
                                    (socklen_t *) &address_len);
                
                int port_number = ntohs(temp.sin_port);
                User* user = users_return(port_number);

                if((sent_size = read(fd_client, buffer_in, BUFSIZE)) == 0){ // disconnct
                    print("- host dissconnected on port : ");
                    print(itoa(port_number, 10)); print("\n");

                    user->online = false;
                    user->busy   = false;
                    FD_CLR(fd_client, &fds_set);

                    close(fd_client);
                    fds[i] = 0;
                }
                else{
                    buffer_in[sent_size] = '\0';
                    Request request;
                    request.user = user;
                    parse_request(&request, buffer_in);
                    do_request(&request, fd_client);
                }
            }
        }
    }
    close(fd_server);
}

void test(){
    char *msg = "shit";
    printf("%d\n", char_to_state(msg));
    users_init(9002);
    users_init(2000);
    users_init(91);
    users_init(124);
    User* user = users_return(91);
    strcpy(user->name, "kiarash norouzi");
    printf("%s on port %d is : %d\n", user->name, user->port, user->online);
    users_delete(user);
    printf("%s on port %d is : %d\n", user->name, user->port, user->online);
    strcpy(user->name, "ali has");
    printf("%s on port %d is : %d\n", user->name, user->port, user->online);
}

int main(int argc, char* argv[]){
    if(argc != 2){
        print("# :/ port please ! \n");
        exit(-1);
    }
    run_server(atoi(argv[1]));
    // test();
    return 0;
}