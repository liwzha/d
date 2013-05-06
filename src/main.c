/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  main() code for chirc project
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */
#include <signal.h>



#include "user.h"
#include "channel.h"
#include "message.h"
#include "server_behavior.h"
#include "simclist.h"

#define BACKLOG 10

void *accept_clients();

struct workerArgs
{
    int socket;
    struct hostent *clientHost;
    struct hostent *serverHost;
    int setup_done;
    
};

struct serverThreadArgs
{
    char* port;
};

void *service_single_client(void *args);

void sigchld_handler(int s){
    while(waitpid(-1,NULL,WNOHANG) > 0 );
}

pthread_mutex_t lock;
pthread_cond_t cv_setup_done;


// user_list can be modified by any process;
//list_t user_list;

int main(int argc, char *argv[])
{
    int opt;
	char *port = "6667", *passwd = NULL;
    
    pthread_t server_thread;
    sigset_t new;
    
	while ((opt = getopt(argc, argv, "p:o:h")) != -1)
        switch (opt)
    {
        case 'p':
            port = strdup(optarg);
            break;
        case 'o':
            passwd = strdup(optarg);
            break;
        default:
            printf("ERROR: Unknown option -%c\n", opt);
            exit(-1);
    }
    
    if (!passwd)
    {
        fprintf(stderr, "ERROR: You must specify an operator password\n");
        exit(-1);
    }

    serverPasswd = strdup(passwd);
    
    
    sigemptyset(&new);
    sigaddset(&new, SIGPIPE);
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cv_setup_done, NULL);
    
    if(pthread_sigmask(SIG_BLOCK,&new,NULL)!=0)
    {
        perror("Unable to mask SIGPIPE");
        exit(-1);
    }
    
    
    
    list_init(&user_list);
    list_init(&onlineUser_list);
    list_init(&channel_list);
    connectionCounter = 0;
    //Create Server Thread;
    
    struct serverThreadArgs *serverArgs;
    
    serverArgs= malloc(sizeof(struct serverThreadArgs));
    serverArgs -> port = port;
    
    
    if(pthread_create(&server_thread,NULL,accept_clients,serverArgs)<0)
    {
        perror("could not create server thread");
        exit(-1);
    }

    pthread_join(server_thread,NULL);
    
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cv_setup_done);
    
    pthread_exit(NULL);
    
    return 0;
}

void *accept_clients(void *args)
{
    struct serverThreadArgs *serverArgs = (struct serverThreadArgs*)args;
    char *serverport = serverArgs->port;
    int serverSocket;
    //    struct addrinfo hints, *servinfo, *p;
    
    
    
    struct sockaddr_in serverAddr;
    
    
    //struct sockaddr_storage clientAddr; // connector's address information
    
    socklen_t sinSize;
    struct sigaction sa;
    struct hostent *serverHost;
    
    
    int yes=1;
    
    char hostNameServer[512];
    
    user_info* server;
    
    list_init(&user_list);// list of user_info *
    
    
    
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(serverport));
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    server = create_user("","","","bar.example.com",serverSocket); //TODO: find server hostname on run-time
	
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    listen(serverSocket, BACKLOG);
    
    printf("server: waiting for connections...\n");
    /* ****** main accept() loop ****** */
    while(1) {
        
        
        struct sockaddr_in clientAddr;
        
        sinSize = sizeof clientAddr;
        
        int clientSocket;
        
        clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
        
        connectionCounter++;
        if (clientSocket == -1) {
            perror("accept");
            continue;
            
        }
        
        pthread_t worker_thread;
        struct workerArgs *wa;
        
        struct hostent *clientHost;
        gethostname(hostNameServer,sizeof(hostNameServer));
        serverHost = gethostbyname(hostNameServer);
        clientHost = gethostbyaddr((char *)&clientAddr.sin_addr.s_addr, sizeof(clientAddr.sin_addr.s_addr), AF_INET);
        
        //Connection Build: Then spawn a new thread;
        
        wa = malloc(sizeof(struct workerArgs));
        wa ->socket = clientSocket;
        wa->clientHost = clientHost;
        wa->serverHost = serverHost;
        
        pthread_mutex_lock(&lock);
        if(pthread_create(&worker_thread,NULL,service_single_client,wa)!=0)
        {
            perror("Could not create a worker thread;");
            pthread_exit(NULL);
            
        }
         
        while(!wa->setup_done)
            pthread_cond_wait(&cv_setup_done, &lock);
 
        pthread_mutex_unlock(&lock);
       // if (!fork()) { // this is the child process??Not sure about the function of fork??
       //     close(serverSocket);
            
       // }
    }
    pthread_exit(NULL);    

    return 0;
}


void *service_single_client(void *args) {
printf("inside service_single_client\n");
    
    int rv, numbytes, buf_offset=0, msg_offset=-1,flag;
    enum cmd_name cmd;
    char *buf, *msg, *prefix, *nick, *username, *fullname, *hostname;
    buf = (char*)malloc(sizeof(char)*MAX_MSG_LEN);
    msg = (char*)malloc(sizeof(char)*MAX_MSG_LEN);
    list_t param_list;
    user_info *usr;
	int socket, nbytes;
	char tosend[100];
    
	/* Unpack the arguments */
    struct workerArgs *wa;
	wa = (struct workerArgs*) args;
	int clientSocket = wa->socket;
    struct hostent *clientHost = wa -> clientHost;
    struct hostent *serverHost = wa -> serverHost;
    
    
	/* This tells the pthreads library that no other thread is going to
     join() this thread. This means that, once this thread terminates,
     its resources can be safely freed (instead of keeping them around
     so they can be collected by another thread join()-ing this thread) */
	
    pthread_detach(pthread_self());
    
	fprintf(stderr, "Socket %d connected\n", clientSocket);
    wa->setup_done = 1;
    pthread_mutex_lock(&lock);
    pthread_cond_signal(&cv_setup_done);
    pthread_mutex_unlock(&lock); 
    
    usr=create_user("","","",clientHost->h_name,clientSocket);
    /* add the user toe the userlist, accquire a lock here
     */
    
    
    /* *** expect for user's connection *** */
    while(1){
        printf("main recv loop....\n"); 
        
        printf("about to call recv_msg\n");
        recv_msg(clientSocket,buf,&buf_offset,msg,&msg_offset );
        printf("returned from recv_msg\n");
        printf("msg:%s\n",msg);
        cmd_message parsed_msg = parse_message(msg);
        printf("returned from parse_message, about to call resp_to_cmd\n");

        pthread_mutex_lock(&lock);
	printf("Inside the lock");
        resp_to_cmd(usr, parsed_msg,serverHost->h_name);
        pthread_mutex_unlock(&lock);
	printf("Outside the lock");
    
    }
    
    close(socket);
    pthread_exit(NULL);
}
