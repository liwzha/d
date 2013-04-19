#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simclist.h"
#include "reply.h"

#define MAX_MSG_LEN 512

enum cmd_name {NICK, USER, WHOIS, MODE, JOIN, PRIVMSG};

typedef struct{
    enum cmd_name c_m_command;// list of type char
    list_t c_m_parameters; 
    //parameter are: if m_command is NICK, m_parameters[1]=user's nick;
    // if m_command is USER, m_parameters[1]=user name, m_parameter[4]= user's full name, m_param[2,3]=*
    // if m_command is PRIVMSG, m_parameters[1]=nick of recepient user. m_parameter[2] = message

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
