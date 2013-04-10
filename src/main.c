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

#define BACKLOG 10
void sigchld_handler(int s){
       while(waitpid(-1,NULL,WNOHANG) > 0 );
}

int main(int argc, char *argv[])
{
	int opt;
	char *port = "7776", *passwd = NULL;
        int serverSocket, clientSocket;
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr_in serverAddr, clientAddr;
        //struct sockaddr_storage clientAddr; // connector's address information
        socklen_t sinSize;
        struct sigaction sa;
        struct hostent *serverHost, *clientHost;
        int yes=1;
        int rv, numbytes, buf_offset=0, msg_offset=-1,flag; 
        enum cmd_name cmd;
        char *buf, *msg, *prefix, *nick, *username, *fullname, *hostname;
        char hostNameServer[512];
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

        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(7776);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        listen(serverSocket, BACKLOG);

        printf("server: waiting for connections...\n");
        /* ****** main accept() loop ****** */
        while(1) {
              sinSize = sizeof clientAddr;
              clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &sinSize);
              if (clientSocket == -1) {
                  perror("accept");
                  continue;
              }

              gethostname(hostNameServer,sizeof(hostNameServer));
              serverHost = gethostbyname(hostNameServer);
              clientHost = gethostbyaddr((char *)&clientAddr.sin_addr.s_addr, sizeof(clientAddr.sin_addr.s_addr), AF_INET);

              if (!fork()) { // this is the child process
                  close(serverSocket);
        
                    /* *** expect for user's connection *** */
                  while(1){

                      recv_msg( clientSocket,buf,&buf_offset,msg,&msg_offset );
                      printf("msg:%s\n",msg);
                      list_init(&param_list);
                      cmd = parse_message(msg,&prefix, &param_list);
                      if( cmd == NICK ){
                          nick = strdup((char *)list_get_at( &param_list, 0));
                          if( find_by_nick( &user_list, nick) != NULL ){
                            // TODO: report nick already exists
                          } 
                      }
                      else if( cmd == USER ){
                          username = list_get_at( &param_list, 0 );
                          fullname = list_get_at( &param_list, 3 );
                          hostname = strdup( clientHost->h_name ); 
                          usr = create_user( nick, username, fullname, hostname ); 
                          list_append( &param_list, &usr);   
                          buf = con_rpl_welcome( server, usr );
                          send_rpl( clientSocket, buf );
                          close_clientSocket( clientSocket );
                      }
                   }
                }
                close(clientSocket);
        }
   
	return 0;
}
