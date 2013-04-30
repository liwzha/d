#ifndef _USER_H
#define _USER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "simclist.h"
#include "message.h"
#include "reply.h"

typedef struct {
    char *ui_nick;
    char *ui_username;
    char *ui_fullname;
    char *ui_hostname;
    int ui_socket;
    // TO DO MODES
    int awayMode;
    int operatorMode;
    char * awayMessage;
} user_info;

// maintain a list of pointers to user_info
list_t user_list; // list of user_info
list_t onlineUser_list; //list of user_info
int connectionCounter;
//list_t channel_list; //list of channel_info

void print(user_info* usr);
//void printlist(list_t user_list);
user_info * init_user();
// create a user_info struct
user_info * create_user( char *nick, char *username, char *fullname, char *hostname, int socket);
// construct the string <nick>!<user>@<host> for user
// OR construct the string <host> for server
char * con_userinfo_str( user_info* user );
bool isempty(user_info* user);
bool is_user_registered(user_info* user);// check if both username and nick are present

user_info* list_find_socket(int socket);//find the user by its socket in the user list
bool is_nick_present(char* nick);// check if nick is already taken
bool is_uname_present(char* uname);//check if username is taken
user_info* list_find_nick(char *nick);
int checkRegisteredUsersNum();


#endif
