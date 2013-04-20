#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simclist.h"
#include "reply.h"

#define MAX_MSG_LEN 512

enum cmd_name {NICK, USER, MODE, JOIN, PRIVMSG, NOTICE, MOTD, LUSERS, PING, PONG, WHOIS, UNKNOWNCOMMAND};

typedef struct{
    enum cmd_name c_m_command;// list of type char
    list_t c_m_parameters; 
    // parameter are: 
    // if c_m_command is NICK, c_m_parameters[0]=user's nick;
    // if c_m_command is USER, c_m_parameters[0]=user name, c_m_parameter[3]= user's full name, c_m_param[1,2]=*
    // if c_m_command is PRIVMSG, c_m_parameters[0]=nick of recepient user. c_m_parameter[1] = message
    // if c_m_command is NOTICE, c_m_parameters[0]=nick of recepient user. c_m_parameter[1] = message
    // if c_m_command is PING, c_m_parameters[0]=* 
    // if c_m_command is PONG, c_m_parameters[0]=* 

}cmd_message;

// assuming message is complete
// return -1 if fail to parse the message
// commenting out the old interface
//enum cmd_name parse_message(char * msg, char ** prefix, list_t* param_list);
// here's the new one:
cmd_message parse_message(char *msg);

// process a raw message received from client
// raw_msg will be left with anything remaining after '\r\n'
// len indicate the length of raw_msg
int extract_message(char *buf, int* buf_offset, int len,  char *msg, int * msg_offset); 

// convert string to command
// string needs to be null terminated
enum cmd_name str2cmd( char *str );

#endif
