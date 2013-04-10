#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simclist.h"
#include "reply.h"

#define MAX_MSG_LEN 512

enum cmd_name {NICK, USER, WHOIS, MODE, JOIN};
// assuming message is complete
// return -1 if fail to parse the message
enum cmd_name parse_message(char * msg, char ** prefix, list_t* param_list);

// process a raw message received from client
// raw_msg will be left with anything remaining after '\r\n'

// len indicate the length of raw_msg
int extract_message(char *buf, int* buf_offset, int len,  char *msg, int * msg_offset); 

// convert string to command
// string needs to be null terminated
enum cmd_name str2cmd( char *str );

#endif
