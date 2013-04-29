#include "channel.h"
void circulate_in_channel(channel_info* chan, char* message){
    // TO DO take care of modes for @ sign
    //channel_info *chan = find_channel_by_nick(channel_nick); 
    user_info *usr = (user_info*)malloc(sizeof(user_info)); 
    int i;
    for(i=0;i<list_size(&chan->ci_users);i++){
	usr = (user_info *)list_get_at( &chan->ci_users, i);
        send_rpl(usr->ui_socket, message);
    }
}
bool is_channel_empty(channel_info* channel){
    if(list_size(&channel->ci_users)==0)
	return 1;
    else 
	return 0;
}

bool is_user_on_channel( channel_info* channel, user_info* sender){
    user_info *usr = (user_info*)malloc(sizeof(user_info)); 
    int i;
    for(i = 0; i < list_size( &channel->ci_users); i++){
	usr = (user_info *)list_get_at( &channel->ci_users, i);
	if(usr->ui_socket==sender->ui_socket){	
	    return 1;
        }
    }
    return 0;
}
char * all_users_channel(char* nick){
    channel_info *chan = find_channel_by_nick(nick);
    user_info *usr = (user_info*)malloc(sizeof(user_info));  
    char buffer[MAX_MSG_LEN];
    int i;
    int cx=0;
    for(i=0;i<list_size(&chan->ci_users);i++){
	usr = (user_info *)list_get_at(& chan->ci_users, i);
        cx = snprintf ( buffer + cx, MAX_MSG_LEN -cx, "%s ", usr->ui_nick);
    }
    buffer[strlen(buffer)-1]='\0';
    char *all_users=strdup(buffer);
    return all_users;
}



channel_info* find_channel_by_nick(char* nick){
    channel_info *chan = (channel_info*)malloc(sizeof(channel_info)); 
    channel_info* p_chan = init_channel(nick);
    int i;
    for(i=0;i<list_size(&channel_list);i++){
	chan = (channel_info *)list_get_at( &channel_list, i);
	if(strcmp(chan->ci_nick,nick)==0){	
	    p_chan=chan;
	    break;
        }
    }
    return p_chan;
}
channel_info* init_channel(char* nick ){
    channel_info *chan = (channel_info*)malloc(sizeof(channel_info));
    chan->ci_nick = strdup(nick);
    list_init(&chan->ci_users);
    list_append(&channel_list,chan);
    return chan;
}

