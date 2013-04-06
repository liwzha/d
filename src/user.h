#ifndef _USER_H
#define _USER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct {
    char *ui_nick;
    char *ui_username;
    char *ui_fullname;
    char *ui_hostname;
} user_info;

// create a user_info struct 
user_info create_user( char *nick, char *username, char *fullname, char *hostname);

// construct the string <nick>!<user>@<host> for user 
// OR construct the string <host> for server
char * con_userinfo_str( user_info user );

#endif
