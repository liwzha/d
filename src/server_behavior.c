#include "server_behavior.h"

void close_clientSocket( int clientSocket ){
    close(clientSocket);
    exit(0);
}

user_info* find_by_nick( list_t *userlist, char * nick_query ){
    int i;
    user_info * p_user;
    if( userlist == NULL )
        return NULL;
    for( i=0; i<list_size(userlist); i++ ){
         p_user = (user_info *) list_get_at(userlist, i);
         if( strcmp( p_user->ui_nick, nick_query )==0 )
             return p_user;
    }
    return NULL;
}

char* con_rpl_welcome( user_info server, user_info usr ){
    char* rpl = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl,"%s %d %s :Welcome to the Internet Relay Network %s!%s@%s",
      con_userinfo_str(server),
      RPL_WELCOME,
      usr.ui_nick,
      usr.ui_nick,
      usr.ui_username,
      usr.ui_hostname); 
    return rpl;
}

void send_rpl( int clientSocket, char* msg ){
    int slen;
    char *msg_to_send;
    slen = strlen(msg);
    msg_to_send = malloc(sizeof(char)*slen+1);
    strcat( msg_to_send, msg );
    msg_to_send[slen-1] = '\r';
    msg_to_send[slen] = '\n';
    send(clientSocket, msg_to_send, slen+1, 0);
}
