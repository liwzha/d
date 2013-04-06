#include "message.h"

int extract_message(char **raw_msg, int len, char **msg){
    int i,j;
    if( len<=0 )
        fprintf(stderr,"extract_message: raw message has length zero!!!\n");
    for( i=1; i<len && !((*raw_msg)[i-1] == '\r' && (*raw_msg)[i] == '\n' ); i++ )
      ;
    *msg = (char*) malloc(sizeof(char)*i);
    for( j=0; j<i-1; j++ )
{
        (*msg)[j] = (*raw_msg)[j];
}
    for( j=i+1; j<len; j++ )
        (*raw_msg)[j-i-1] = (*raw_msg)[j];
    (*raw_msg)[len-i-1] = '\0';
    (*msg)[i-1] = '\0';
    return i<len;
}

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
enum cmd_name parse_message(char *msg, char **prefix, list_t *param_list){
    list_init(param_list);
    char * substring, *cmd=NULL, *tmp;
    if( strlen(msg) == 0 ){
        fprintf(stderr,"parse_message: msg has length zero!!!\n");
        exit(0);
    } 
    parse_message_helper(msg,param_list,1);
    tmp = (char *)list_get_at( param_list,0 );
    if( tmp[0]==':' ){
        *prefix = strdup( tmp );
        list_delete_range( param_list,0,0 );
    }
    else
        *prefix = strdup("");
    cmd = (char *)list_get_at( param_list,0 );
    list_delete_range( param_list,0,0 );
    return str2cmd( cmd ); 
}

enum cmd_name str2cmd( char *str ){
    if( strcmp( str, "NICK" ) == 0 )
        return NICK;
    else if( strcmp( str, "USER" ) == 0 )
        return USER;
    else{
        fprintf(stderr,"cannot recognize command %s\n",str);
        return -1;// if command does not exist
    }
}
