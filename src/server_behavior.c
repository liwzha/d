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
    sprintf(rpl,"%s\r\n%s\r\n%s\r\n%s", 
            rpl1,
	    rpl2,
            rpl3,
            rpl4);
//":hostname 251 user1 :There are 1 users and 0 services on 1 servers\r\n:hostname 252 user1 0 :operator(s) online\r\n:hostname 253 user1 0 :unknown connection(s)\r\n:hostname 254 user1 0 :channels formed\r\n:hostname 255 user1 :I have 1 clients and 1 servers\r\n:hostname 422 user1 :MOTD File is missing"
//);
    return rpl;
}

void send_rpl( int clientSocket, char* msg ){
    int slen;
    char *msg_to_send;
    slen = strlen(msg);
    printf("send_rpl: about to send msg (len=%d,socket=%d):\n%s\n#EOM#\n",strlen(msg),clientSocket,msg);
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
printf("about to call recv..............\n");
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
    case LUSERS:
            printf("*******************case LUSERS *****************\n");
            rpl_lusers(usr, &parsed_msg, serverHost);
            break;
	case QUIT:
            send_quit(usr, parsed_msg, serverHost);
            break;
	case JOIN:
            send_join(usr, parsed_msg, serverHost);
	    break;
    case PART:
            send_part(usr, parsed_msg, serverHost);
            break;
    case TOPIC:
            rpl_topic(usr, &parsed_msg, serverHost);
    case OPER:
            rpl_oper(usr, &parsed_msg, serverHost);
    case MODE:
            rpl_mode(usr,&parsed_msg, serverHost);
    case AWAY:
            rpl_away(usr, &parsed_msg, serverHost);
            
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
	    list_append(&onlineUser_list,check_usr);
               
	    char * buffer ;
	    buffer = con_rpl_welcome( serverhost, check_usr );
	    send_rpl( clientSocket, buffer );
            rpl_lusers(check_usr, NULL, serverhost);
            rpl_motd(check_usr, NULL, serverhost);
         }
         else
	     check_usr->ui_nick= strdup(nick);
    }
    /*printf("\n-----------------------------------------------------\n");
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
    printf("\n-----------------------------------------------------\n");*/
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
printf("----->>>>>>about to add a user by name<<<<<<<<---------\n");
printf("current number of user:%d\n",list_size(&onlineUser_list));
	check_usr->ui_username = strdup(username);	
	if(strlen(full_username)!=0)
	    check_usr->ui_fullname=full_username;
	list_append(&onlineUser_list,check_usr);
printf("appended---------->>>current number of user:%d<<<-------------\n",list_size(&onlineUser_list));
	char *buffer;
	buffer = con_rpl_welcome( serverhost, usr );
        send_rpl( clientSocket, buffer );
        rpl_lusers(check_usr, NULL, serverhost);
        rpl_motd(check_usr, NULL, serverhost);
    }
    else{		
	if(strlen(full_username)!=0)
	    usr->ui_fullname=full_username;
    }
    /*printf("\n-----------------------------------------------------\n");
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
    printf("\n-----------------------------------------------------\n");*/
}
void send_join(user_info* usr, cmd_message parsed_msg, char* serverHost){
    char* channel_nick=strdup((char *)list_get_at( &parsed_msg.c_m_parameters, 0));   
    channel_info* chan=find_channel_by_nick(channel_nick); // returns
    
    char *all_users=all_users_channel(channel_nick); 
    char join_msg[MAX_MSG_LEN]; //JOINING MESSAGE 
    char out_buf1[MAX_MSG_LEN]; //RPL TOPIC
    char out_buf2[MAX_MSG_LEN]; //RPL NAMREPLY   
    char out_buf3[MAX_MSG_LEN]; //PRL_ENDOFNAMES     
    if( is_channel_empty (chan)){
        chan->ci_nick = strdup(channel_nick);
        chan->topicSet = 0;
        list_init(&chan->ci_users);
	list_append(&channel_list,chan);
    } 
    if( !is_user_on_channel (chan, usr)){
    	list_append(&chan->ci_users, usr);
    }
    printf("\n\n-----------------------------------------------------\n");
    printf("send_join: Number of channels in channel_list %d", list_size(&channel_list));    
    int i;
    for(i=0;i<list_size(&channel_list);i++){
        channel_info* print_chan = (channel_info *)list_get_at( &channel_list, i);
	    //if(!is_channel_empty(print_chan)){
		printf("\n:%s:%d",
                print_chan->ci_nick,
                list_size(&print_chan->ci_users));
	    //}    
    }
    printf("\n-----------------------------------------------------\n");
    sprintf(join_msg,":%s!%s@%s JOIN %s", 
			usr->ui_nick
   		      , usr->ui_username
                      , usr->ui_hostname
                      , channel_nick );
    circulate_in_channel(chan,join_msg);
    if(chan->topicSet){
        sprintf(out_buf1,":%s %s %s %s :%s",
	  		serverHost,
			RPL_TOPIC,
			usr->ui_nick,
			channel_nick,
			chan->topic);
	send_rpl( usr->ui_socket, out_buf1);
    }
	//RPL_NAMREPLY
    sprintf(out_buf2,":%s %s %s = %s :%s foobar2",
			serverHost, 
			RPL_NAMREPLY, 
			usr->ui_nick,
                        channel_nick,
                        usr->ui_nick);
    send_rpl( usr->ui_socket, out_buf2 );
        // RPL_ENDOFNAMES    
    sprintf(out_buf3,":%s %s %s %s :End of NAMES list", 
			serverHost,
			RPL_ENDOFNAMES,
		        usr->ui_nick,
                        channel_nick);
    send_rpl( usr->ui_socket, out_buf3 );
}

// TO DO 
void send_part(user_info* usr, cmd_message parsed_msg, char* serverHost){
    char buffer [MAX_MSG_LEN];
    list_t * param = &(parsed_msg.c_m_parameters);
    char* channel_nick = list_get_at(param, 0);
    channel_info* chan=find_channel_by_nick(channel_nick);
    if(chan == NULL){
        sprintf(buffer,":%s %s", serverHost
                                ,  ERR_NOSUCHCHANNEL);
    }
    else if (is_user_on_channel(chan, usr)){        
        sprintf(buffer,":%s %s", serverHost
                               , ERR_NOTONCHANNEL);
    }
    else{
        list_delete(&chan->ci_users, usr);
        if(strlen((char *)list_get_at( param, 1))!=0){
             sprintf(buffer,":%s %s!%s@%s PART %s :%s", serverHost
                                  , usr->ui_nick
				  , usr->ui_username
                                  , usr->ui_hostname
				  , channel_nick
                                  , (char *)list_get_at( param, 1));
             }
        else{
    	     sprintf(buffer,":%s %s!%s@%s PART %s", serverHost
                                  , usr->ui_nick
				  , usr->ui_username
                                  , usr->ui_hostname
				  , channel_nick);
        }
        printf("Message to be sent:\n%s\nTo socket %d\n",buffer,usr->ui_socket);
        circulate_in_channel(chan, buffer );// TO DO circulate
        if(is_channel_empty(chan)){
	    list_delete(&channel_list, chan);
        }
    }    
}
void send_private_message(user_info *usr, cmd_message parsed_msg, char* serverHost,enum cmd_name command){
    list_t * param = &(parsed_msg.c_m_parameters);
    char* nick = list_get_at(param, 0);
    //check if the receiver is a channel or a user
    if( nick[0]=='#' || nick[0]=='!' || nick[0]=='&' || nick[0]=='+'){
	channel_info *chan = find_channel_by_nick(nick); 
	if( is_channel_empty (chan) || chan==NULL){
		char buffer [MAX_MSG_LEN];
		snprintf ( buffer, sizeof(buffer),
			":%s %s %s %s :No such nick/channel",
			serverHost,
			ERR_NOSUCHNICK, 
			usr->ui_nick,
			nick);
                send_rpl( usr->ui_socket, buffer );
	}
	else if( chan->moderateMode == 2 ){// moderate mode =m
	    if( is_user_voice_user(chan, usr)){
	    	user_info *usr_2 = (user_info*)malloc(sizeof(user_info)); 
            	int i;
            	for(i=0;i<list_size(&chan->ci_users);i++){
	        	usr_2 = (user_info *)list_get_at( &chan->ci_users, i);
			if(strcmp(usr_2->ui_nick,usr->ui_nick) !=0){
				list_delete_at(&(parsed_msg.c_m_parameters),0);
				list_prepend(&(parsed_msg.c_m_parameters),usr_2->ui_nick); 
        	        	send_private_message_usr(usr, parsed_msg, serverHost,command,nick);
			}
            	}
	    }
	    else{
		char buffer [MAX_MSG_LEN];
		sprintf(buffer,":%s %s", serverHost,  ERR_CANNOTSENDTOCHAN);
                send_rpl( usr->ui_socket, buffer );
	    }
	}
	else{ 
            user_info *usr_2 = (user_info*)malloc(sizeof(user_info)); 
            int i;
            for(i=0;i<list_size(&chan->ci_users);i++){	        
	        usr_2 = (user_info *)list_get_at( &chan->ci_users, i);
		if(strcmp(usr_2->ui_nick,usr->ui_nick) !=0){
			list_delete_at(&(parsed_msg.c_m_parameters),0);
			list_prepend(&(parsed_msg.c_m_parameters),usr_2->ui_nick);            
        	        send_private_message_usr(usr, parsed_msg, serverHost,command,nick);
		}
            }
	}	
    }
    else{
        send_private_message_usr(usr, parsed_msg, serverHost,command,nick);
    }
}

void send_private_message_usr(user_info *usr, cmd_message parsed_msg, char* serverHost,enum cmd_name command, char* channel_name){
    user_info *receiver=NULL;
    printf("----------------------------------------------------------------\n");
    printf("Inside send private message usr: NICK %s\n",
           (char *)list_get_at( &parsed_msg.c_m_parameters, 0));
    receiver=list_find_nick(list_get_at( &parsed_msg.c_m_parameters, 0 ));

    if(isempty(receiver)|| !is_user_registered(usr) || !is_user_registered(receiver)){
	if(command==PRIVMSG && command!=NOTICE){
	    char buffer [MAX_MSG_LEN];
            snprintf ( buffer, sizeof(buffer),
			":%s %s %s %s :No such nick/channel",
			serverHost,
			ERR_NOSUCHNICK, 
			usr->ui_nick,
			channel_name);
	   // printf("Message to be sent:\n%s\nTo socket %d\n",buffer,usr->ui_socket);
            send_rpl( usr->ui_socket, buffer );
	}
    }	
    else{
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
                channel_name,  
                (char*)list_get_at( &parsed_msg.c_m_parameters, 1 ));
	 //printf("Message to be sent:\n%s\nTo socket %d\n",buffer,receiver->ui_socket);
 	 //printf("msg:%s\nlenght=%d\n",(char*)list_get_at( &parsed_msg.c_m_parameters, 1 ),strlen((char*)list_get_at( &parsed_msg.c_m_parameters, 1 )));
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
    char* nick = sender_info->ui_nick;//list_get_at(param, 0);
    serverHost = malloc(sizeof(char)*30);
    strcat(serverHost,"hostname"); //!!!!!!!!!!!!!!!!for debug!!!!!!!!!!!!!!!!!!!!11 
    if(fr == NULL)    {
        fprintf(stderr,"Can't open motd file!\n");
        fileExist = false;
    }
    else    {
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

void rpl_lusers(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost)
{
printf("----->> inside rpl_lusers << ------\n");
    int userSock = sender_info->ui_socket;
    char out_buf[MAX_MSG_LEN];
    int numChannels = 0;
    int registeredUsers = 0;
    char* nick = sender_info->ui_nick;
 
serverHost = malloc(sizeof(char)*10);
strcat(serverHost,"hostname\0"); //!!!!!!!!!!!!!!!!for debug!!!!!!!!!!!!!!!!!!!!11 
    
    //RPL_LUSERCLIENT number of registered users
    registeredUsers = checkRegisteredUsersNum();
    sprintf(out_buf, ":%s %s %s :There are %d users and 0 services on 1 servers", serverHost, RPL_LUSERCLIENT,nick, registeredUsers);
    send_rpl(userSock, out_buf);

    //RPL_LUSEROP  number of users online
    sprintf(out_buf, ":%s %s %s %d :operator(s) online", serverHost, RPL_LUSEROP, nick, 0);
    send_rpl(userSock, out_buf);
    
    //RPL_LUSERUNKNOWN  number of connections unknown
    int unknownConnections = connectionCounter - registeredUsers;
    sprintf(out_buf, ":%s %s %s %d :unknown connection(s)", serverHost, RPL_LUSERUNKNOWN, nick, unknownConnections);
    send_rpl(userSock, out_buf);
    
    //RPL_LUSERCHANNELS
    if(&channel_list == NULL)
        numChannels = 0;
    else
        numChannels = list_size(&channel_list);
    sprintf(out_buf, ":%s %s %s %d :channels formed",serverHost, RPL_LUSERCHANNELS,nick, numChannels);
    send_rpl(userSock, out_buf);
    
    //pthread_mutex_lock(&connectionlock);
    //RPL_LUSERME   all number of connections(online + unknown)
    sprintf(out_buf, ":%s %s %s :I have %d clients and 1 servers", serverHost, RPL_LUSERME, nick, connectionCounter);
    send_rpl(userSock, out_buf);
    //pthread_mutex_unlock(&connectionlock);
}

void rpl_topic(user_info* sender_info, cmd_message* p_parsed_msg, char* serverHost)
{
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char * nick = sender_info->ui_nick;
    char * channelName = list_get_at(param, 0);
    char out_buf[MAX_MSG_LEN];
    channel_info *channel = find_channel_by_nick(channelName);
    if(!is_user_on_channel(find_channel_by_nick(channelName), sender_info))
    {
        sprintf(out_buf, ":%s %s %s %s :You're not on that channel", serverHost,ERR_NOTONCHANNEL,nick,channelName);
        send_rpl(userSock, out_buf);
        return;
    }
    if(list_size(param) == 2)
    {
        if(!channel->topicSet)
        {
            sprintf(out_buf,":%s %s %s %s :No topic is set", serverHost, RPL_NOTOPIC,nick,channelName);
            send_rpl(userSock, out_buf);
            return;
        }
        else
        {
            sprintf(out_buf,":%s %s %s %s :%s",serverHost,RPL_TOPIC,nick,channelName,channel->topic);
            send_rpl(userSock,out_buf);
            return;
        }
    }
    else
    {
        /*
         Check priviledge;
         */
        if(channel->topicMode == 1 && !list_contains(&(channel->ci_operatorUsers),sender_info) && sender_info->operatorMode != 1)
        {
            sprintf(out_buf, "%s %s %s %s :You're not channel operator", serverHost, ERR_CHANOPRIVSNEEDED, nick, channelName);
            send_rpl(userSock, out_buf);
            return;
        }
        else
        {
            
            char* newChannelName = list_get_at(param,2);
            if(strlen(newChannelName) == 0)//Clear the Topic Name
            {
                channel->topicSet = 0;
                channel->topic = "";
                //Do we need to send any command?
                return;
            }
            else
            {
                channel->topicSet = 1;
                channel->topic = newChannelName;
                //Do we need to send any command?
                return;
            }
        }
    }
}

void rpl_oper(user_info * sender_info, cmd_message * p_parsed_msg, char* serverHost)
{
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char * nick = sender_info->ui_nick;
    char * userPasswd = list_get_at(param, 1);
    char out_buf[MAX_MSG_LEN];
    
    if(strcmp(userPasswd, serverPasswd) == 0)
    {
        user_info *user = list_find_nick(nick);
        user->operatorMode = 1;
        sprintf(out_buf, "%s %s %s:You are now an IRC operator",serverHost,RPL_YOUREOPER,nick);
        send_rpl(userSock, out_buf);
        return;
    }
    else
    {
        sprintf(out_buf, "%s %s %s:Password incorrect",serverHost,ERR_PASSWDMISMATCH,nick);
        send_rpl(userSock, out_buf);
        return;
    }
}

void rpl_mode(user_info * sender_info, cmd_message * p_parsed_msg, char* serverHost)
{
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char * nick = sender_info->ui_nick;
    char * userName = sender_info->ui_username;
    char out_buf[MAX_MSG_LEN];
    char * name = list_get_at(param, 0);
    if(name[0]!='#' && name[0]!='&' && name[0]!='!' && name[0]!='+') // User Mode:----Weird Method to Determine whether it's a user mode or not.
    {
        if(strcmp(nick,name)!=0)
        {
            sprintf(out_buf, "%s %s %s:Cannot change mode for other users",serverHost,ERR_USERSDONTMATCH,nick);
            send_rpl(userSock, out_buf);
            return;
        }
        else
        {
           if(strcmp(list_get_at(param, 1),"-o"))
           {
               user_info * user = list_find_nick(nick);
               user->operatorMode = 0;
               sprintf(out_buf, "%s :%s %s %s :-o",serverHost,nick,"MODE",nick);
               send_rpl(userSock, out_buf);
               return;
           }
            else
            {
                sprintf(out_buf, "%s %s %s:Unknown MODE flag", serverHost, ERR_UMODEUNKNOWNFLAG,nick);
                send_rpl(userSock, out_buf);
                return;
            }
        }
    }
    
    else    //Channel Mode
    {
        if(list_size(param) == 1)//Channel Mode with one parameter
        {
            if(!is_channel_on_list(name))
            {
                sprintf(out_buf, "%s %s %s %s :No such channel", serverHost,ERR_NOSUCHCHANNEL,nick,name);
                send_rpl(userSock, out_buf);
                return;
            }
            else
            {
                channel_info *channel = find_channel_by_nick(name);
                if(channel->topicMode + channel->moderateMode == 2)
                {
                    sprintf(out_buf, "%s %s %s %s +%s %s",serverHost,RPL_CHANNELMODEIS,nick,name,"t","m");
                    send_rpl(userSock, out_buf);
                    return;
                }
                else if(channel->topicMode == 1)
                {
                    sprintf(out_buf, "%s %s %s %s +%s",serverHost,RPL_CHANNELMODEIS,nick,name,"t");
                    send_rpl(userSock, out_buf);
                    return;
                }
                else if(channel->moderateMode == 1)
                {
                    sprintf(out_buf, "%s %s %s %s +%s",serverHost,RPL_CHANNELMODEIS,nick,name,"m");
                    send_rpl(userSock, out_buf);
                    return;
                }
                    else // What if there's no mode?
                        return;
            }
        }
        
        else if(list_size(param) == 2)//Channel Mode with two parameters;
        {
            if(!is_channel_on_list(name))
            {
                sprintf(out_buf, "%s %s %s %s :No such channel",serverHost,ERR_NOSUCHCHANNEL,nick,name);
                send_rpl(userSock, out_buf);
                return;
            }
            else
            {
                channel_info *channel = find_channel_by_nick(name);
                char * modeString = list_get_at(param, 1);
                //check priviledge
                if(list_contains(&(channel->ci_operatorUsers),sender_info)||sender_info->operatorMode)
                {
                    if(modeString[1]!='m' && modeString[1]!='t')//Unknown Mode
                    {
                        sprintf(out_buf, "%s %s %s %c :is unknown mode char to me for %s", serverHost,ERR_UNKNOWNMODE,nick,modeString[1],name);
                        send_rpl(userSock, out_buf);
                        return;
                    }
                    if(modeString[1] == 'm')
                    {
                        if(modeString[0] == '+')
                            channel->moderateMode = 1;
                        if(modeString[0] == '-')
                            channel->moderateMode = 0;
                    }
                    if(modeString[1] == 't')
                    {
                        if(modeString[0] == '+')
                            channel->topicMode = 1;
                        if(modeString[0] == '-')
                            channel->topicMode = 0;
                    }
                    /**********************************
                     TODO: Send Message to the user and All the Users in the channel;
                     
                     
                     ************************************/
                }
                else
                {
                    sprintf(out_buf, "%s %s %s %s :You're not channel operator",serverHost,ERR_CHANOPRIVSNEEDED,nick,name);
                    send_rpl(userSock, out_buf);
                    return;
                }
            
            }
        }
        
        else if(list_size(param) == 3)//Member Status Modes;
        {
            if(!is_channel_on_list(name))
            {
                sprintf(out_buf, "%s %s %s %s :No such channel",serverHost,ERR_NOSUCHCHANNEL,nick,name);
                send_rpl(userSock, out_buf);
                return;
            }
            channel_info *channel = find_channel_by_nick(name);
            char * nickNameOfModifiedUser = list_get_at(param, 2);
            char * modeString = list_get_at(param, 1);
            //check priviledge
            if(!list_contains(&(channel->ci_operatorUsers),sender_info)&& !sender_info->operatorMode)
            {
                sprintf(out_buf, "%s %s %s %s :You're not channel operator",serverHost,ERR_CHANOPRIVSNEEDED,nick,name);
                send_rpl(userSock, out_buf);
                return;
            }
            else if(!is_user_on_channel(channel, list_find_nick(nickNameOfModifiedUser)))
            {
                sprintf(out_buf, "%s %s %s %s %s :They aren't on that channel",serverHost,ERR_USERNOTINCHANNEL,nick,nickNameOfModifiedUser,name);
                send_rpl(userSock, out_buf);
                return;
            }
            else if(modeString[1]!='o' && modeString[1]!='v')
            {
                sprintf(out_buf, "%s %s %s %c :is unknown mode char to me for %s", serverHost,ERR_UNKNOWNMODE,nick,modeString[1],name);
                send_rpl(userSock, out_buf);
                return;
            }
            else
            {
                if(modeString[0] == '+')
                {
                    if(modeString[1] == 'o')
                        list_append(&(channel->ci_operatorUsers),list_find_nick(nickNameOfModifiedUser));
                    if(modeString[1] == 'v')
                        list_append(&(channel->ci_voiceUsers), list_find_nick(nickNameOfModifiedUser));
                }
                if(modeString[0] == '-')
                {
                    if(modeString[1] == 'o')
                        list_delete(&(channel->ci_operatorUsers), list_find_nick(nickNameOfModifiedUser));
                    if(modeString[1] == 'v')
                        list_delete(&(channel->ci_voiceUsers), list_find_nick(nickNameOfModifiedUser));
                }
                /*********************
                 TODO: Send reply to the User and all user in that channel;
                 *********************/
            }
        }
        
        else
            return;
    }
}

void rpl_away(user_info * sender_info, cmd_message * p_parsed_msg, char* serverHost)
{
    list_t * param = &(p_parsed_msg->c_m_parameters);
    int userSock = sender_info->ui_socket;
    char * nick = sender_info->ui_nick;
    char out_buf[MAX_MSG_LEN];
    if(list_size(param)==0)
    {
        sender_info->awayMode = 0;
        sender_info->awayMessage = "";
        sprintf(out_buf, "%s %s %s:You are no longer marked as being away",serverHost,RPL_NOWAWAY,nick);
        send_rpl(userSock, out_buf);
        return;
    }
    else if(list_size(param)==1)
    {
        sender_info->awayMode = 1;
        sender_info->awayMessage = list_get_at(param, 0);
        sprintf(out_buf, "%s %s %s:You have been marked as being away", serverHost, RPL_AWAY,nick);
        send_rpl(userSock, out_buf);
        return;
    }
    else//no else;
        return;
}



