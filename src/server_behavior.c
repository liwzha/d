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

char* con_rpl_welcome( char *server, user_info *usr ){
    char* rpl = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl,"%s %s %s :Welcome to the Internet Relay Network %s!%s@%s",
            server,
            RPL_WELCOME,
            usr->ui_nick,
            usr->ui_nick,
            usr->ui_username,
            usr->ui_hostname);
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
printf("inside recv_msg, buf_offset = %d, msg_offset = %d\n",*buf_offset, *msg_offset);
    if( (*buf_offset)==0 ){
        if(( numbytes = recv( clientSocket, buf, MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    } else
        numbytes = (*buf_offset);
    while(( flag=extract_message(buf,buf_offset,numbytes,msg,msg_offset))==-1 ){
printf("recv_msg:  incomplete msg, waiting for rest of the msg from usr...\n");
        if(( numbytes = recv( clientSocket, buf+(*buf_offset), MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    }
    
}

void resp_to_cmd(user_info *usr, cmd_message parsed_msg, char* serverHost){
//    cmd_message parsed_msg = parse_message(msg);//convert raw message to struct cmd_message
    switch ( parsed_msg.c_m_command) {
        case NICK:
            add_user_by_nick((char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),
                             usr,
                             serverHost);
            break;
        case USER:
            add_user_by_uname((char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),
                              (char*)list_get_at( &parsed_msg.c_m_parameters, 3 ),
                              usr,
                              serverHost);
            break;
        case PRIVMSG:
 		send_private_message(usr, parsed_msg, serverHost);
            break;
        default:
            // TO DO: ERR_UNKNOWNCOMAND
            break;
    }
}


void add_user_by_nick(char* nick, user_info *usr, char* serverhost){
        int clientSocket=usr->ui_socket;
        if(is_nick_present(nick)){// Nick already present, send error
            char buffer [MAX_MSG_LEN];
            snprintf ( buffer, sizeof(buffer),
               ":%s %s * %s :Nickname is already in use",
               serverhost,
               ERR_NICKNAMEINUSE,
               nick);
            send_rpl( clientSocket, buffer );
        }
        else {
            user_info *check_usr=NULL;
            check_usr = list_find_socket(clientSocket);// searches for socket
			//printf("\nadd %d\n",2);
			//print(*check_usr);
			//printf("\n:username:%s:\n",strlen((*check_usr).ui_username));
            if (isempty(check_usr)){
                usr->ui_nick=nick;
                list_append(&user_list,usr);
            }
            else if(strlen(check_usr->ui_username)!=0 && !is_user_registered(usr)){
			//welcome and add nick
                (*check_usr).ui_nick=malloc(sizeof(char)*strlen(nick));
		strcpy((*check_usr).ui_nick,nick);
		list_append(&user_list,&check_usr);
                
		char * buffer ;
		buffer = con_rpl_welcome( serverhost, check_usr );
		send_rpl( clientSocket, buffer );
				//printf("\nadd %d\n",4);
	     }
	     else{
	         check_usr->ui_nick=malloc(sizeof(char)*strlen(nick));
		 strcpy((*check_usr).ui_nick,nick);
		 list_append(&user_list,&check_usr);
				//printf("\nadd %d\n",5);
             }
	}
	//printlist(*user_list);
}

void add_user_by_uname(char* username,char* full_username,user_info *usr,char* serverhost){
		user_info *check_usr=NULL;
		int clientSocket=usr->ui_socket;
		check_usr = list_find_socket(clientSocket);// searches for socket
		//printf("\nadd %d\n",2);
		//print(*check_usr);
		if (isempty(check_usr)){
			usr->ui_username=username;
			if(strlen(full_username)!=0)
				usr->ui_fullname=full_username;
			list_append(&user_list,usr);
		}
		else if(strlen((*check_usr).ui_nick)!=0 && !is_user_registered(usr) ){
			(*check_usr).ui_username=malloc(sizeof(char)*strlen(username));
			strcpy((*check_usr).ui_username,username);
			if(strlen(full_username)!=0)
				check_usr->ui_fullname=full_username;
			list_append(&user_list,&check_usr);
			//New username is added
			char *buffer;
			buffer = con_rpl_welcome( serverhost, usr );
			strcpy(buffer,"Welcome");
            send_rpl( clientSocket, buffer );
		}
		else{
			(*check_usr).ui_username=malloc(sizeof(char)*strlen(username));
			strcpy((*check_usr).ui_username,username);
			if(strlen(full_username)!=0)
				usr->ui_fullname=full_username;
			list_append(&user_list,&check_usr);
		}
	//printlist(*user_list);
}

void send_private_message(user_info *usr, cmd_message parsed_msg, char* serverHost){
	user_info *receiver=NULL;
	receiver=list_find_nick((char*)list_get_at( &parsed_msg.c_m_parameters, 0 ));

	if(isempty(receiver)){
		//TO DO ERROR NO NICK
		printf("no nick");
	}	
	else if (is_user_registered(usr) && is_user_registered(receiver)){
           char buffer [MAX_MSG_LEN];
           snprintf ( buffer, sizeof(buffer),
                    ":%s!%s@%s\nPRIVMSG %s :%s",
                     usr->ui_nick,
                     usr->ui_username,
                     usr->ui_hostname,
                     (char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),  
                     (char*)list_get_at( &parsed_msg.c_m_parameters, 1 ));
           send_rpl(receiver->ui_socket, buffer );
        }
        else{
              // TO DO:If user unregistered: show error
		printf("User Unregistered");
        }
}

