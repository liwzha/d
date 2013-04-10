/*
 *
 *  CMSC 23300 / 33300 - Networks and Distributed Systems
 *
 *  main() code for chirc project
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "user.h"
#include "message.h"
#include "server_behavior.h"
#include "simclist.h"

void sigchld_handler(int s){
       while(waitpid(-1,NULL,WNOHANG) > 0 );
}

int main(int argc, char *argv[])
{
	int opt;
	char *port = "7776", *passwd = NULL;
        int serverSocket, clientSocket;
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr_in serverAddr;
        struct sockaddr_storage clientAddr; // connector's address information
        socklen_t sinSize;
        struct sigaction sa;
        int yes=1;
        int rv, numbytes, buf_offset=0, msg_offset=-1,flag; 
        enum cmd_name cmd;
        char *buf, *msg, *prefix, *nick, *username, *fullname, *hostname, *char_p;
        user_info usr, server;
        list_t user_list, param_list;
        list_init(&user_list);// list of user_info *
        server = create_user("","","","bar.example.com"); //TODO: find server hostname on run-time
        buf = (char*)malloc(sizeof(char)*MAX_MSG_LEN);  
        msg = (char*)malloc(sizeof(char)*MAX_MSG_LEN);  
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

        /* ****** setting up server socket ****** */
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
              fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
              return 1;
        }
        for (p = servinfo; p != NULL; p = p->ai_next) {
              if ((serverSocket = socket(p->ai_family, p->ai_socktype,
                          p->ai_protocol)) == -1) {
                    perror("server: socket");
                    continue;
              }
      
              if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,
                          sizeof(int)) == -1){
                    perror("setsockopt");
                    exit(1);
              }
             
              if (bind(serverSocket, p->ai_addr,p->ai_addrlen) == -1) {
                    close(serverSocket);
                    perror("server: bind");
                    continue;
              }
              break;
        }

        if (p==NULL) {
              fprintf(stderr, "server: failed to bind\n");
              return 2; 
        }
         
        freeaddrinfo(servinfo); // all done with this structure

        if (listen(serverSocket, 5) == -1) {
              perror("listen");
              exit(1);
        }

        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }


        printf("server: waiting for connections...\n");
        /* ****** main accept() loop ****** */
        while(1) {
              sinSize = sizeof clientAddr;
              clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
              if (clientSocket == -1) {
                  perror("accept");
                  continue;
              }


              // TODO: get host name
              /*inet_ntop(clientAddr.ss_family,
              ((struct sockaddr_in*)clientAddr)->sin_addr,
              s, sizeof s);
              clientHostName = strdup(gethostbyaddr(s,4,AF_INET)); */
              if (!fork()) { // this is the child process
                    close(serverSocket);
        
                    /* *** expect for user's connection *** */
                    recv_msg( clientSocket,buf,&buf_offset,msg,&msg_offset );
printf("msg:%s\n",msg);
                    list_init(&param_list);
                    cmd = parse_message(msg,&prefix, &param_list);
                    if( cmd != NICK )
                        close_clientSocket(clientSocket);
                    nick = strdup((char *)list_get_at( &param_list, 0));
                    if( find_by_nick( &user_list, nick) != NULL ){
// TODO: report nick already exists
} 
                     
                    recv_msg(clientSocket,buf,&buf_offset,msg,&msg_offset);
printf("msg:%s\n",msg);
                    list_init(&param_list);
                    cmd = parse_message( msg, &prefix, &param_list );
                    if( cmd != USER )
                        close_clientSocket(clientSocket);
                    username = list_get_at( &param_list, 0 );
                    fullname = list_get_at( &param_list, 3 );
                    hostname = strdup( "foo.example.com" ); //TODO: find user's host name in run-time
                    usr = create_user( nick, username, fullname, hostname ); 
                    list_append( &param_list, &usr);   
                    buf = con_rpl_welcome( server, usr );
                    send_rpl( clientSocket, buf );
                    close_clientSocket( clientSocket );

              }
              close(clientSocket);
        }

	return 0;
}
