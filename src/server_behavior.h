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
#include <time.h> 
#include "simclist.h"
#include "user.h"
#include "message.h"
#include "reply.h"
#include "channel.h"

char * serverPasswd;
// close a client socket and exit thread
void close_clientSocket( int clientSocket );

// find user's info in a list by nick
//user_info* find_by_nick( list_t *userlist, char * nick_query );

// construct welcome info
char* con_rpl_welcome( char *server, user_info *usr );

// send reply to client.  remove '\0' at the end of msg and append \r\n.
void send_rpl( int clientSocket, char* msg );

// reveive message from client
void recv_msg(int clientSocket, char *buf, int *buf_offset, char *msg, int *msg_offset );

// design the response for a raw message
void resp_to_cmd(user_info *usr, cmd_message msg, char* serverHost);

// add user while checking if nick is already present
void add_user_by_nick(char* nick,user_info *usr,char* serverhost);

// add by username
void add_user_by_uname(char* username,char* full_username,user_info *usr, char* serverhost);

// send private message in commands PRIVMSG and NOTICE
void send_private_message(user_info *usr, cmd_message parsed_msg, char* serverHost,enum cmd_name command);

// send private message in commands PRIVMSG and NOTICE
void send_private_message_usr(user_info *usr, cmd_message parsed_msg, char* serverHost,enum cmd_name command, char* channel_name);
// send PONG in response to PING
void send_pong(user_info *usr, char* serverHost);

// send reply to whois command.  report error if nick does not exist.
void rpl_whois(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

// send err_unknowncommand if the command can't be recognized
void rpl_unknowcommand(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

//send quit message
void send_quit(user_info* usr, cmd_message parsed_msg, char* serverHost);

void rpl_motd(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void rpl_lusers(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void rpl_topic(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void rpl_oper(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void rpl_mode(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void rpl_away(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost);

void send_join(user_info* usr, cmd_message parsed_msg, char* serverHost);

void send_part(user_info* usr, cmd_message parsed_msg, char* serverHost);


#endif

