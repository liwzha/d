#include "user.h"


user_info * init_user(){
    return create_user("","","","",-1);
}

bool isempty(user_info *user){
    // check if the user is empty
    if(user->ui_socket<=0)//TO DO doesnt handle null
	return 1;
    else
	return 0;
}

bool is_user_registered(user_info* user){
    //check if the user is not empty and both user name and nick are present
    if(isempty(user))
	return 0;
    else if(strlen(user->ui_nick)==0 || strlen(user->ui_username)==0)
        return 0;
    else
        return 1;
}

bool is_nick_present(char* nick){
    //check if a particular  nick is present in the list
    bool value=0;
    int i;
    user_info usr;
    for(i=0;i<list_size(&user_list);i++){
	usr = *(user_info *)list_get_at( &user_list, i);
	if(strcmp(usr.ui_nick,nick)==0){
	    value=1;
	    return 1;
	}		
    }
    return value;
}
bool is_uname_present(char* uname){
    //check if a particular  uname is present in the list
    bool value=0;
    int i;
    user_info usr;
    for(i=0;i<list_size(&user_list);i++){
	usr = *(user_info *)list_get_at( &user_list, i);
	if(strcmp(usr.ui_username,uname)==0){
	    value=1;
	    return 1;
	}		
    }
    return value;
}

user_info *create_user( char *nick, char *username, char *fullname, char *hostname, int socket){
    //allocate memory to the new user
    user_info *user = (user_info*)malloc(sizeof(user_info));
    if( !( nick && username && fullname && hostname ) ){
        fprintf(stderr,"*error* create_user: augments cannot be NULL. (use empty string "" instead.)\n");
        exit(1);
    }
    user->ui_nick = strdup(nick);
    user->ui_username = strdup(username);
    user->ui_fullname = strdup(fullname);
    user->ui_hostname = strdup(hostname);
    user->ui_socket   = socket;
    return user;
}

char *con_userinfo_str( user_info* user ){
    // create a string of the userhost name
    int size = 1 + strlen(user->ui_nick) + strlen(user->ui_username) + strlen(user->ui_hostname);
    char *info_str=NULL;
    size += (strlen(user->ui_nick) == 0)? 0:2;
    info_str = (char *)malloc( sizeof(char)*size + 1 );
    if( strlen(user->ui_nick) > 0 )
        sprintf(info_str,":%s!%s@%s",user->ui_nick,user->ui_username,user->ui_hostname);
    else
        sprintf(info_str, ":%s",user->ui_hostname);
    return info_str;
}

user_info* list_find_socket(int socket){
    //find the user corresponding to a particular socket in the list
    user_info *usr = (user_info*)malloc(sizeof(user_info)); 
    user_info* p_usr = init_user();
    int i;
    for(i=0;i<list_size(&user_list);i++){
	usr = (user_info *)list_get_at( &user_list, i);
	if(usr->ui_socket==socket){	
	    p_usr=usr;
	    break;
        }
    }
    return p_usr;
}

user_info* list_find_nick(char *nick){
    //find the user corresponding to a particular nick in the list
    user_info *usr = (user_info*)malloc(sizeof(user_info));    
    user_info* p_usr = init_user();
    int i;
    for(i=0;i<list_size(&user_list);i++){
	usr = (user_info *)list_get_at( &user_list, i);
	if(strcmp(usr->ui_nick,nick)==0){
	    p_usr=usr;
            break;
	}
    }
    return p_usr;
}

void print(user_info* usr){
    // print a particular user
    if(!isempty(usr)){
	printf("\n--Inside user.h--:%s:%s:%s:%s:%d\n",
            usr->ui_nick,
            usr->ui_username,
            usr->ui_fullname,
            usr->ui_hostname,
            usr->ui_socket);
	}
    else{
	printf("\nEmpty USER\n");
    }
}
/*void printlist(list_t user_list){
	printf("\nPrintlist");
	user_info usr;
	int i;
	printf("\n________________________\n");
	for(i=0;i<list_size(&user_list);i++){
		usr = *(user_info *)list_get_at( &user_list, i);
		if(!isempty(usr)){
			printf("\n:%s:%s:%s:%s:%d",
                   usr.ui_nick,
                   usr.ui_username,
                   usr.ui_fullname,
                   usr.ui_hostname,
                   usr.ui_socket);
		}
		else{
			printf("\nEmpty USER\n");
		}
	}
	printf("\n________________________\n");
}*/

