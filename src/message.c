#include "message.h"

// *msg_offset -- index of last valid charactor stored in msg
// *buf_offset -- index of first newly received char stored in buf 
// len -- length of newly received message
// return 0: if exact one complete msg
// return 1: if contains more than one complete msg
// return -1: if msg incomplete
int extract_message(char *buf, int* buf_offset, int len, char *msg, int*msg_offset){
printf("inside extract message...processing raw msg (len = %d, buf_offset = %d):\n",len,*buf_offset);
int t;
for( t=0;t<len;t++ )
    printf("%c",buf[t]);
printf("\n");

    int i,j,flag,reader=0;
    len += (*buf_offset);// now, len should be equal to the length of total elements in buf 
    if( len<=0 )
        fprintf(stderr,"extract_message: raw message has length zero!!!\n");
    for( ; (*msg_offset)<1; )
        msg[++(*msg_offset)] = buf[reader++];

    for( ; reader < len && !(msg[(*msg_offset)-1] == '\r' && msg[*msg_offset] == '\n'); reader++){
        msg[++(*msg_offset)] = buf[reader];
    }

    if(msg[(*msg_offset)-1] == '\r' && msg[*msg_offset] == '\n'){
        if( reader==len ){ // if exact a complete msg
            flag = 0;
        }
        else{ // if more than a complete msg
            flag = 1;
            for(i=0;i<(len-reader);i++)
                buf[i] = buf[i+reader];
        }
        msg[(*msg_offset)-1] = '\0';
        (*buf_offset) = len-reader;
        (*msg_offset) = -1;
    }
    else{// if msg is incomplete
        flag = -1;
        //(*msg_offset)++;
        *buf_offset = 0;
    }
printf("extract_message: about to return\n");
    return flag;
}

// helper function for parse_message
// extract the first substring before a space
// delete the substring from msg
void parse_message_helper(char *msg, list_t *param_list, int is_front){
    char *substring, *tmp;
    if( strlen( msg ) == 0 )
        return;
    if( !is_front && msg[0]==':' )// if is a long parameter
    {
        list_append(param_list, strdup(msg) );
        return;
    }
    substring = strchr( msg, ' ' );
    if( substring != NULL ){
        *substring = '\0';
    }
    tmp = strdup( msg );
    list_append(param_list, tmp );
    if( substring != NULL )
        parse_message_helper( substring+1, param_list, 0 ); 
}


cmd_message parse_message(char *msg){
    char *cmd = NULL;
    cmd_message cm;
    if( strlen(msg) == 0 ){
        fprintf(stderr,"parse_message: msg has length zero!!!\n");
        exit(0);
    } 
    list_init(& cm.c_m_parameters);
    parse_message_helper(msg, &cm.c_m_parameters,1);
    cmd = (char *)list_get_at( &cm.c_m_parameters,0 );

    cm.c_m_command = str2cmd( cmd );
    if( cm.c_m_command != UNKNOWNCOMMAND )
        list_delete_range( &cm.c_m_parameters,0,0 );
    return cm; 
}

enum cmd_name str2cmd( char *str ){
    if (     strcmp( str,"NICK"   ) == 0 )
        return NICK;
    else if( strcmp( str,"USER"   ) == 0)
        return USER;
    else if( strcmp( str, "PRIVMSG" ) == 0 )
        return PRIVMSG;
    else if( strcmp( str, "NOTICE" ) == 0 )
        return NOTICE;
    else if( strcmp( str, "MOTD" ) == 0 )
        return MOTD;
    else if( strcmp( str, "LUSERS" ) == 0 )
        return LUSERS;
    else if( strcmp( str, "PING" ) == 0 )
        return PING;
    else if( strcmp( str, "PONG" ) == 0 )
        return PONG;
    else if( strcmp( str, "WHOIS" ) == 0 )
        return WHOIS;
    else if( strcmp( str, "QUIT" ) == 0 )
        return QUIT;
    else if( strcmp( str, "JOIN" ) == 0 )
        return JOIN;
    else if( strcmp( str, "PART" ) == 0 )
        return PART;
    else if( strcmp( str, "TOPIC" ) == 0 )
        return TOPIC;
    else if( strcmp( str, "AWAY" ) == 0 )
        return AWAY;
    else if( strcmp( str, "NAMES" ) == 0 )
        return NAMES;
    else if( strcmp( str, "LIST" ) == 0 )
        return LIST;
    else if( strcmp( str, "WHO" ) == 0 )
        return WHO;
    else if( strcmp( str, "OPER" )== 0 )
        return OPER;
    else if( strcmp( str, "MODE") == 0)
        return MODE;
    else{
        fprintf(stderr,"*************cannot recognize command %s*****************\n",str);
        return UNKNOWNCOMMAND;// if command does not exist
    }
}
