#include "message.h"

// return 0: if exact one complete msg
// return 1: if contains more than one complete msg
// return -1: if msg incomplete
int extract_message(char *buf, int* buf_offset, int len, char *msg, int*msg_offset){
printf("inside extract message...processing raw msg (len = %d):\n",len);
int t;
for( t=0;t<len;t++ )
    printf("%c",buf[t]);
printf("\n");

    int i,j,flag,reader=0;
    len += (*buf_offset); 
    if( len<=0 )
        fprintf(stderr,"extract_message: raw message has length zero!!!\n");
    if( (*msg_offset) == -1 ){
        msg[++(*msg_offset)] = buf[reader++];
        printf("%c",buf[reader-1]);
    }
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
    list_delete_range( &cm.c_m_parameters,0,0 );
    cm.c_m_command = str2cmd( cmd );
    return cm; 
}

// commenting out the old parse_message function
//enum cmd_name parse_message(char *msg, char **prefix, list_t *param_list){
//printf("parse_message in msg=%s\n",msg);
//    list_init(param_list);
//    char * substring, *cmd=NULL, *tmp;
//    if( strlen(msg) == 0 ){
//        fprintf(stderr,"parse_message: msg has length zero!!!\n");
//        exit(0);
//    } 
//    parse_message_helper(msg,param_list,1);
//    tmp = (char *)list_get_at( param_list,0 );
//    if( tmp[0]==':' ){
//        *prefix = strdup( tmp );
//        list_delete_range( param_list,0,0 );
//    }
//    else
//        *prefix = strdup("");
//    cmd = (char *)list_get_at( param_list,0 );
//    list_delete_range( param_list,0,0 );
//    return str2cmd( cmd ); 
//}

enum cmd_name str2cmd( char *str ){
    if (     strcmp( str,"NICK"   ) == 0 )
        return NICK;
    else if( strcmp( str,"USER"   ) == 0)
        return USER;
    else if( strcmp( str,"PRIVMSG") == 0)
	return PRIVMSG;
    else if( strcmp( str,"NOTICE" ) == 0)
	return NOTICE;
    else if( strcmp( str,"PING" ) == 0)
	return PING;
    else if( strcmp( str,"PONG" ) == 0)
	return PONG;
    else{
        fprintf(stderr,"cannot recognize command %s\n",str);
        return -1;// if command does not exist
    }
}
