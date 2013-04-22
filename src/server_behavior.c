#include "server_behavior.h"

void close_clientSocket( int clientSocket ){
    close(clientSocket);
    exit(0);
}

char* con_rpl_welcome( char *server, user_info *usr ){
    char* rpl1 = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl1,":%s %s %s :Welcome to the Internet Relay Network %s!%s@%s",
            server,
            RPL_WELCOME,
            usr->ui_nick,
            usr->ui_nick,
            usr->ui_username,
            usr->ui_hostname
	    );
    
    char* rpl2 = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl2,":%s %s %s :Your host is %s, running version %s",
            server,
            RPL_YOURHOST,
            usr->ui_nick,
            server,
  	    "version1"// TO DO 
	    );
    
    time_t timer;
    struct tm y2k;
    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

    time(&timer); 
    char* rpl3 = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl3,":%s %s %s :This server was created %d/%d/%d",
            server,
            RPL_CREATED,
            usr->ui_nick,
            y2k.tm_mon,
	    y2k.tm_mday,
            y2k.tm_year
	    );
  
    char* rpl4 = malloc( sizeof(char)*MAX_MSG_LEN );
    sprintf(rpl4,":%s %s %s %s %s %s %s",
            server,
            RPL_MYINFO,
            usr->ui_nick,
            server,
	    "version1", // TO DO
            "ao", 
 	    "mtov"
            );  
       
    char* rpl = malloc( sizeof(char)*MAX_MSG_LEN*2 );
    sprintf(rpl,"%s\r\n%s\r\n%s\r\n%s\r\n%s", //the last string is just for testing!!!!!
            rpl1,
	    rpl2,
            rpl3,
            rpl4,
":hostname 251 user1 :There are 1 users and 0 services on 1 servers\r\n:hostname 252 user1 0 :operator(s) online\r\n:hostname 253 user1 0 :unknown connection(s)\r\n:hostname 254 user1 0 :channels formed\r\n:hostname 255 user1 :I have 1 clients and 1 servers\r\n:hostname 422 user1 :MOTD File is missing");
    return rpl;
}

void send_rpl( int clientSocket, char* msg ){
    int slen;
    char *msg_to_send;
    slen = strlen(msg);
    printf("send_rpl: about to send msg:\n%s\n#EOM#\n",msg);
    msg_to_send = malloc(sizeof(char)*slen+2);
    strcat( msg_to_send, msg );
    msg_to_send[slen] = '\r';
    msg_to_send[slen+1] = '\n';
    send(clientSocket, msg_to_send, slen+2, 0);
    //free(msg_to_send);
}

void recv_msg( int clientSocket, char *buf, int *buf_offset, char *msg, int *msg_offset ){
    int numbytes,flag;
    printf("inside recv_msg, buf_offset = %d, msg_offset = %d\n",*buf_offset, *msg_offset);
    if( (*buf_offset)==0 ){
        if(( numbytes = recv( clientSocket, buf, MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    } else
        numbytes = 0;

    while(( flag=extract_message(buf,buf_offset,numbytes,msg,msg_offset))==-1 ){
    printf("recv_msg:  incomplete msg, waiting for rest of the msg from usr...\n");
        if(( numbytes = recv( clientSocket, buf+(*buf_offset), MAX_MSG_LEN, 0 )) == -1 )
            perror("recv");
    }
    
}

void resp_to_cmd(user_info *usr, cmd_message parsed_msg, char* serverHost){
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
 	    send_private_message(usr, parsed_msg, serverHost,PRIVMSG);
            break;
	case NOTICE:
            send_private_message(usr, parsed_msg, serverHost,NOTICE);
            break;
	case PING:
 	    send_pong(usr,serverHost);
            break;
	case PONG:
 	    // DO NOTHING
            break;
        case WHOIS:
            rpl_whois(usr, &parsed_msg, serverHost);
            break;
        case MOTD:
            rpl_motd(usr, &parsed_msg, serverHost);
            break;
	case QUIT:
            send_quit(usr, parsed_msg, serverHost);
            break;
        default:
            rpl_unknowcommand(usr, &parsed_msg, serverHost);
            break;
    }
}


void add_user_by_nick(char* nick, user_info *usr, char* serverhost){
    int clientSocket=usr->ui_socket;
    if(is_nick_present(nick)){
        // Nick already present, send error
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
        if (isempty(check_usr)){
            usr->ui_nick=nick;
            list_append(&user_list,usr);
        }
    	else if(strlen(check_usr->ui_username)!=0 && !is_user_registered(usr)){ 
            //welcome and add nick
            check_usr->ui_nick= strdup(nick);	
	    list_append(&onlineUser_list,&check_usr);
               
	    char * buffer ;
	    buffer = con_rpl_welcome( serverhost, check_usr );
	    send_rpl( clientSocket, buffer );
         }
         else
	     check_usr->ui_nick= strdup(nick);
    }
    printf("\n-----------------------------------------------------\n");
    printf("NICK:Number of elements in the list %d", list_size(&user_list));
    int i;
    for(i=0;i<list_size(&user_list);i++){
		usr = (user_info *)list_get_at( &user_list, i);
		if(!isempty(usr)){
			printf("\n:%s:%s:%s:%s:%d",
                   usr->ui_nick,
                   usr->ui_username,
                   usr->ui_fullname,
                   usr->ui_hostname,
                   usr->ui_socket);
		}    
    }
    printf("\n-----------------------------------------------------\n");
}

void add_user_by_uname(char* username,char* full_username,user_info *usr,char* serverhost){
    user_info *check_usr=NULL;
    int clientSocket=usr->ui_socket;
    check_usr = list_find_socket(clientSocket);// searches for socket
    printf("Add user by username: Client socket=%d\n",clientSocket);
    
    if (isempty(check_usr)){
	usr->ui_username=username;
	if(strlen(full_username)!=0)
	    usr->ui_fullname=full_username;
	list_append(&user_list,usr);
    }
    else if(strlen((*check_usr).ui_username)!=0){
        // Username already present for that socket
        char buffer [MAX_MSG_LEN];
        snprintf ( buffer, sizeof(buffer), 
		   "%s %s", 
		   serverhost,
                   ERR_ALREADYREGISTRED);
        send_rpl( clientSocket, buffer );
    }
    else if(strlen((*check_usr).ui_nick)!=0 && !is_user_registered(usr) ){
	// user already has a nick, with the username it should get a Welcome reply since it gets completely registered now
	check_usr->ui_username = strdup(username);	
	if(strlen(full_username)!=0)
	    check_usr->ui_fullname=full_username;
	list_append(&onlineUser_list,&check_usr);
	char *buffer;
	buffer = con_rpl_welcome( serverhost, usr );
        send_rpl( clientSocket, buffer );
    }
    else{		
	if(strlen(full_username)!=0)
	    usr->ui_fullname=full_username;
    }
    printf("\n-----------------------------------------------------\n");
    printf("Uname: Number of elements in the list %d", list_size(&user_list));    
    int i;
    for(i=0;i<list_size(&user_list);i++){
		usr = (user_info *)list_get_at( &user_list, i);
		if(!isempty(usr)){
			printf("\n:%s:%s:%s:%s:%d",
                   usr->ui_nick,
                   usr->ui_username,
                   usr->ui_fullname,
                   usr->ui_hostname,
                   usr->ui_socket);
		}    
    }
    printf("\n-----------------------------------------------------\n");
}

void send_private_message(user_info *usr, cmd_message parsed_msg, char* serverHost,enum cmd_name command){
    user_info *receiver=NULL;
    printf("----------------------------------------------------------------\n");
    printf("Inside send private message: NICK %s\n",
           (char *)list_get_at( &parsed_msg.c_m_parameters, 0));
    receiver=list_find_nick(list_get_at( &parsed_msg.c_m_parameters, 0 ));

    if(isempty(receiver)|| !is_user_registered(usr) || !is_user_registered(receiver)){
	if(command==PRIVMSG && command!=NOTICE){
	    char buffer [MAX_MSG_LEN];
            snprintf ( buffer, sizeof(buffer),":%s %s %s %s :No such nick/channel",serverHost,ERR_NOSUCHNICK, usr->ui_nick,(char *) list_get_at( &parsed_msg.c_m_parameters, 0 ));
	    printf("Message to be sent:\n%s\nTo socket %d\n",buffer,usr->ui_socket);
            send_rpl( usr->ui_socket, buffer );
	}
    }	
    else {
	char command_string [MAX_MSG_LEN];
	if(command == PRIVMSG)
	    strcpy(command_string,"PRIVMSG");
        else if(command == NOTICE)
            strcpy(command_string,"NOTICE");
	else
	    exit(1);
        printf("Nick found %s\n",receiver->ui_nick);
        char buffer [MAX_MSG_LEN];
        snprintf ( buffer, sizeof(buffer),
                ":%s!%s@%s %s %s %s",
                usr->ui_nick,
                usr->ui_username,
                usr->ui_hostname,
		command_string,
                (char*)list_get_at( &parsed_msg.c_m_parameters, 0 ),  
                (char*)list_get_at( &parsed_msg.c_m_parameters, 1 ));
	 printf("Message to be sent:\n%s\nTo socket %d\n",buffer,receiver->ui_socket);
 printf("msg:%s\nlenght=%d\n",(char*)list_get_at( &parsed_msg.c_m_parameters, 1 ),strlen((char*)list_get_at( &parsed_msg.c_m_parameters, 1 )));
         send_rpl(receiver->ui_socket, buffer );
    }
    printf("---------------------------------------------------------------- \n");
}

void send_pong(user_info *usr, char* serverHost){
    char buffer [MAX_MSG_LEN];
    snprintf ( buffer, sizeof(buffer),
           "PONG %s",
           serverHost);
    printf("Message to be sent:\n%s\nTo socket %d\n",buffer,usr->ui_socket);
    send_rpl(usr->ui_socket, buffer );
}



void rpl_whois(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost){
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char* nick = list_get_at(param, 0);
    char out_buf[MAX_MSG_LEN];
    if( list_size( param ) > 1 )
        fprintf(stderr,"rpl_whois: receiving more than one parameters!!!!\n");

    user_info* p_nick_owner=NULL;
    p_nick_owner = list_find_nick(nick);
    if( isempty(p_nick_owner) ){
        sprintf(out_buf,":%s %s %s %s :No such nick/channel",
        serverHost, ERR_NOSUCHNICK, sender_info->ui_nick, nick);
        send_rpl( userSock, out_buf );
        return;
    } 
    else {
        sprintf(out_buf,":%s %s %s %s %s %s * %s", serverHost
                                , RPL_WHOISUSER
                                , nick
                                , nick
                                , p_nick_owner->ui_username
                                , p_nick_owner->ui_hostname
                                , p_nick_owner->ui_fullname);
       send_rpl( userSock, out_buf );
    }
    
    // rpl_whoisserver
    sprintf(out_buf,":%s %s %s %s %s :server info", serverHost
                                , RPL_WHOISSERVER
                                , nick
                                , nick
                                , serverHost);
    send_rpl( userSock, out_buf );

    // rpl_endofwhois
    sprintf(out_buf,":%s %s %s %s :End of WHOIS list", serverHost
                               , RPL_ENDOFWHOIS
                               , nick
                               , nick);
    send_rpl( userSock, out_buf );
}

void rpl_unknowcommand(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost){
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char out_buf[MAX_MSG_LEN];
    char *cmd = list_get_at(param,0);
    sprintf(out_buf,":%s %s %s %s :Unknown command", serverHost
                                       , ERR_UNKNOWNCOMMAND
                                       , sender_info->ui_nick
                                       , cmd);
    send_rpl( userSock, out_buf );
}

void send_quit(user_info* usr, cmd_message parsed_msg, char* serverHost){
    char buffer [MAX_MSG_LEN];
    if(strlen((char *)list_get_at( &parsed_msg.c_m_parameters, 0))!=0){
    	snprintf ( buffer, sizeof(buffer),
                "Closing Link: %s %s",
                usr->ui_hostname, 
	        (char *)list_get_at( &parsed_msg.c_m_parameters, 0));
    }
    else{
    	snprintf ( buffer, sizeof(buffer),
                "Closing Link: %s Client quit",
                usr->ui_hostname);
    }
    printf("Message to be sent:\n%s\nTo socket %d\n",buffer,usr->ui_socket);
    send_rpl(usr->ui_socket, buffer );
    list_delete(&user_list, usr);
    connectionCounter--;
}


void rpl_motd(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost){    

    

    int userSock = sender_info->ui_socket;

    char out_buf[MAX_MSG_LEN];

    char messageOfToday[MAX_MSG_LEN];

    FILE *fr = fopen("motd.txt","rt");

    bool fileExist =true;

    list_t * param = &(p_parsed_msg->c_m_parameters);

    char* nick = list_get_at(param, 0);



    if(fr == NULL)

    {

        fprintf(stderr,"Can't open motd file!\n");

        fileExist = false;

    }

    else

    {

        fileExist = true;

    }

    if(!fileExist)

    {

        sprintf(out_buf, ":%s %s %s :MOTD File is missing",serverHost, ERR_NOMOTD, nick);

        send_rpl(userSock, out_buf);

    }

    else

    {

        sprintf(out_buf,":%s %s %s :- %s Message of the day - ",serverHost, RPL_MOTDSTART, nick, serverHost);

        send_rpl(userSock, out_buf);

        

        //Line by Line

        char line[81];

        while(fgets(line, sizeof line, fr)!= NULL)

        {

        sprintf(out_buf, ":%s %s %s :- %s",serverHost, RPL_MOTD, nick, line);

        send_rpl(userSock, out_buf);

        }

        //

        sprintf(out_buf, ":%s %s %s :- End of MOTD command",serverHost, RPL_ENDOFMOTD, nick);

        send_rpl(userSock, out_buf);

        fclose(fr);

    }

}


