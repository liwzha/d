#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "simclist.h"
#include "message.h"
#include "reply.h"
#include "user.h"
typedef struct {
    char *ci_nick;
    list_t ci_users;
    list_t ci_voiceUsers;
    list_t ci_operatorUsers;
    int moderateMode;
    int topicMode;
    char * topic;
    int topicSet;
} channel_info;
// TO DO MODES
// TO DO TOPICS
list_t channel_list; //list of channel_info

void circulate_in_channel( channel_info* channel, char* message);
bool is_channel_empty( channel_info* channel);
bool is_user_on_channel( channel_info* channel, user_info* sender);
char * all_users_channel( char* nick);
bool is_channel_on_list(char* nick);
channel_info* find_channel_by_nick( char* nick);
channel_info* init_channel( char* nick);



#endif
