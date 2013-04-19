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
    sprintf(rpl,"%s %s %s :Welcome to the Internet Relay Network %s!%s@%s",
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

void recv_msg( int clientSocket, char *buf, int *buf_offset, char *msg, int *msg_offset ){
    int numbytes,flag;
    if( (*buf_offset)==0 ){
        if(( numbytes = recv( clientSocket, buf, MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    } else
        numbytes = (*buf_offset);
    while(( flag=extract_message(buf,buf_offset,numbytes,msg,msg_offset))==-1 ){
        if(( numbytes = recv( clientSocket, buf+(*buf_offset), MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    }
    
}

//void resp_to_cmd(user_info usr, char* msg,list_t user_list, char* serverHost){
void resp_to_cmd(user_info usr, cmd_message parsed_msg, char* serverHost){
//    cmd_message parsed_msg = parse_message(msg);//convert raw message to struct cmd_message
    switch ( parsed_msg.c_m_command) {
        case NICK:
            add_user_by_nick((char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),
                             &usr,
                             serverHost);
            break;
        case USER:
            add_user_by_uname((char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),
                              (char*)list_get_at( &parsed_msg.c_m_parameters, 3 ),
                              &usr,
                              serverHost);
            break;
        case PRIVMSG:
           // if (is_user_registered(usr) && is_user_registered(parsed_msg.To_user)){
           //     char buffer [MAX_MSG_LEN];
           //     snprintf ( buffer, sizeof(buffer),
           //               ":%s!%s@%s\nPRIVMSG %s :%s%",
           //               usr.ui_nick,
           //               usr.ui_username,
           //               usr.ui_hostname,
           //              (char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),  
           //              (char*)list_get_at( &parsed_msg.c_m_parameters, 1 ),  
           //               );
           //     send_rpl(parsed_msg.To_user, buffer )
           // }
           // else{
           //     // TO DO:If user unregistered: show error
           // }
            break;
        default:
            // TO DO: ERR_UNKNOWNCOMAND
            break;
    }
}


