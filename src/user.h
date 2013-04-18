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
} user_info;
void print(user_info usr);
void printlist(list_t user_list);
user_info init_user();
// create a user_info struct
user_info create_user( char *nick, char *username, char *fullname, char *hostname, int socket);
user_info init_user();
// construct the string <nick>!<user>@<host> for user
// OR construct the string <host> for server
char * con_userinfo_str( user_info user );
bool isempty(user_info user);
bool is_user_registered(user_info user);// check if both username and nick are present
void add_user_by_nick(char* nick,user_info* usr, list_t* user_list,char* serverhost);//add user while checking if nick is already present
void add_user_by_uname(char* username,char* full_username,user_info* usr, list_t* user_list,char* serverhost);// add by username
void list_find_socket(list_t user_list,int socket,user_info **p_usr);//find the user by its socket in the user list
bool is_nick_present(list_t user_list,char* nick);// check if nick is already taken
#endif
