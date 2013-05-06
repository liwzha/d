#include "channel.h"
void circulate_in_channel(channel_info* chan, char* message){
    // TO DO take care of modes for @ sign
    //channel_info *chan = find_channel_by_nick(channel_nick);
    user_info *usr = (user_info*)malloc(sizeof(user_info));
    int i;
    for(i=0;i<list_size(&chan->ci_users);i++){
        usr = (user_info *)list_get_at( &chan->ci_users, i);
	if(usr->awayMode != 1){
            send_rpl(usr->ui_socket, message);
	}
    }
}
bool is_channel_empty(channel_info* channel){
    if(list_size(&channel->ci_users)==0 || (channel==NULL))
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
bool is_user_voice_user( channel_info* channel, user_info* sender){
    user_info *usr = (user_info*)malloc(sizeof(user_info));
    int i;
    for(i = 0; i < list_size( &channel->ci_voiceUsers); i++){
        usr = (user_info *)list_get_at( &channel->ci_voiceUsers, i);
        if(usr->ui_socket==sender->ui_socket){
            return 1;
        }
    }
    return 0;
}
bool is_user_operator_user( channel_info* channel, user_info* sender){
    user_info *usr = (user_info*)malloc(sizeof(user_info));
    int i;
    for(i = 0; i < list_size( &channel->ci_operatorUsers); i++){
        usr = (user_info *)list_get_at( &channel->ci_operatorUsers, i);
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

bool is_channel_on_list(char* nick){
    int find = 0;
    int i;
    for(i = 0; i <list_size(&channel_list);i++)
    {
        if(strcmp(nick, ((channel_info *)list_get_at(&channel_list, i))->ci_nick)==0)
        {
            find = 1;
            break;
        }
    }
    return find;
}
channel_info* find_channel_by_nick(char* nick){
    channel_info *chan = (channel_info*)malloc(sizeof(channel_info));
    //chan=NULL;
    int i;
    for(i=0;i<list_size(&channel_list);i++){
        chan = (channel_info *)list_get_at( &channel_list, i);
        if(strcmp(chan->ci_nick,nick)==0){
            return chan;
        }
    }
    
    return chan;
}
channel_info* init_channel(char* nick ){
    channel_info *chan = (channel_info*)malloc(sizeof(channel_info));
    chan->ci_nick = strdup(nick);
    chan->topicSet = 0;
    list_init(&chan->ci_users);
    return chan;
}



list_t find_channel_of_user(char *nick){
   int i,j;
   user_info *pt_usr=NULL, *pt_tmpusr = NULL;
   channel_info *pt_chan;
   list_t loc_channel;
   list_init(&loc_channel);

   for( i=0; i<list_size(&user_list); i++ ){
     pt_tmpusr = list_get_at(&user_list, i);
     if( strcmp( pt_tmpusr->ui_nick, nick ) == 0 ){
       pt_usr = pt_tmpusr;
       break;
     }
   }
   
   if( pt_usr == NULL ){
      fprintf(stderr, "find_channel_of_user: cannot find user with the nick: %s\n", nick);  
      exit(1);
   }
   
   for( i=0; i<list_size(&channel_list); i++ ){
     pt_chan = list_get_at(&channel_list, i);
     if( is_user_on_channel( pt_chan, pt_usr ) )
        list_append(&loc_channel, pt_usr);
   }
   return loc_channel;
}

