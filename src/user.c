#include "user.h"

user_info create_user( char *nick, char *username, char *fullname, char *hostname){
    user_info user;
    if( !( nick && username && fullname && hostname ) ){
        fprintf(stderr,"*error* create_user: augments cannot be NULL. (use empty string "" instead.)\n");
        exit(1);
    }
    user.ui_nick = strdup(nick);
    user.ui_username = strdup(username);
    user.ui_fullname = strdup(fullname);
    user.ui_hostname = strdup(hostname);
    return user;
} 

char *con_userinfo_str( user_info user ){
    int size = 1 + strlen(user.ui_nick) + strlen(user.ui_username) + strlen(user.ui_hostname);
    char *info_str;
    size += (strlen(user.ui_nick) == 0)? 0:2;
    info_str = (char *)malloc( sizeof(char)*size + 1 );
    if( strlen(user.ui_nick) > 0 )
        sprintf(info_str,":%s!%s@%s",user.ui_nick,user.ui_username,user.ui_hostname);
    else
        sprintf(info_str, ":%s",user.ui_hostname);
    return info_str;
}
