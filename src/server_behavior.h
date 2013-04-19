#ifndef _SERVER_BEHAVIOR
#define _SERVER_BEHAVIOR

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "simclist.h"
#include "user.h"
#include "message.h"
#include "reply.h"

// close a client socket and exit thread
void close_clientSocket( int clientSocket );

// find user's info in a list by nick
user_info* find_by_nick( list_t *userlist, char * nick_query );

// construct welcome info
char* con_rpl_welcome( user_info server, user_info usr );

// send reply to client.  remove '\0' at the end of msg and append \r\n.
void send_rpl( int clientSocket, char* msg );

void recv_msg(int clientSocket, char *buf, int *buf_offset, char *msg, int *msg_offset );

// design the response for a raw message
//void resp_to_cmd(user_info usr, char* msg,list_t user_list, char* serverHost);
void resp_to_cmd(user_info usr, cmd_message msg, char* serverHost);

#endif

