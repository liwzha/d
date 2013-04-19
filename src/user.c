#include "user.h"
/*
 int main(){
 user_info usr=init_user();
 //printf("%d\n",is_user_registered(usr));
 user_info usr1=create_user("1","2","3","4",1);
 user_info usr2=create_user("a","b","c","d",2);
 //print(usr1);
 list_t user_list;
 list_init(&user_list);
 list_append(&user_list,&usr1);
 list_append(&user_list,&usr2);
 //printf("Check nick:%d\n",is_nick_present(user_list,"0"));
 
 user_info *usr3;
 usr3=(user_info *) malloc(sizeof(user_info *));
 list_find_socket(user_list,2,"n",&usr3);
 //(*usr3).ui_nick="nikku";
 char* nick;
 nick=malloc(sizeof(char)*strlen("nikku"));
 strcpy(nick,"nikku");
 (*usr3).ui_nick=malloc(sizeof(char)*strlen(nick));
 strcpy((*usr3).ui_nick,nick);
 print(*usr3);
 printlist(user_list);
 
 char* nick=NULL;
 nick=malloc(sizeof(char)*strlen("nikku"));
 strcpy(nick,"nikku");
 user_info usr4=create_user("","","","abc",3);
 //printf("\nempty:%d\n", isempty(usr2));
 //add_user_by_nick(nick,&usr4, &user_list,"local_host")	;
 //printlist(user_list);
 //print(*(user_info *)list_get_at( &user_list, 1));
 
 char* user=NULL;
 user=malloc(sizeof(char)*strlen("nikka"));
 strcpy(user,"nikka");
 usr4=create_user("","","","abc",3);
 //printf("\nempty:%d\n", isempty(usr2));
 add_user_by_uname(user,&usr4, &user_list,"local_host")	;
 printlist(user_list);
 return 0;
 }*/
void print(user_info usr){
	if(!isempty(usr)){
		printf("\n--Inside user.h--:%s:%s:%s:%s:%d\n",
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
void printlist(list_t user_list){
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
}
bool isempty(user_info user){
	if(user.ui_socket<=0)//doesnt handle null
		return 1;
	else
		return 0;
}
user_info init_user(){
	return create_user("","","","",-1);
}

user_info create_user( char *nick, char *username, char *fullname, char *hostname, int socket){
    user_info user;
    if( !( nick && username && fullname && hostname ) ){
        fprintf(stderr,"*error* create_user: augments cannot be NULL. (use empty string "" instead.)\n");
        exit(1);
    }
    user.ui_nick = strdup(nick);
    user.ui_username = strdup(username);
    user.ui_fullname = strdup(fullname);
    user.ui_hostname = strdup(hostname);
    user.ui_socket   = socket;
    return user;
}


char *con_userinfo_str( user_info user ){
    int size = 1 + strlen(user.ui_nick) + strlen(user.ui_username) + strlen(user.ui_hostname);
    char *info_str=NULL;
    size += (strlen(user.ui_nick) == 0)? 0:2;
    info_str = (char *)malloc( sizeof(char)*size + 1 );
    if( strlen(user.ui_nick) > 0 )
        sprintf(info_str,":%s!%s@%s",user.ui_nick,user.ui_username,user.ui_hostname);
    else
        sprintf(info_str, ":%s",user.ui_hostname);
    return info_str;
}

bool is_user_registered(user_info user){
    if(strlen(user.ui_nick)==0 || strlen(user.ui_username)==0)
        return 0;
    else
        return 1;
}

void add_user_by_nick(char* nick,user_info* usr, list_t* user_list,char* serverhost){
	if( list_size( user_list)==0){
        list_init( user_list);    // initialize the list
	}
	else{
		int clientSocket=usr->ui_socket;
		if(is_nick_present(*user_list,nick)){
			// Nick already present, send error
  			char buffer [MAX_MSG_LEN];
 			snprintf ( buffer, sizeof(buffer),
                      ":%s %s * %s :Nickname is already in use",
                      serverhost,
                      ERR_NICKNAMEINUSE,
                      nick);
			send_rpl( clientSocket, buffer );
			//printf("\nadd %d\n",1);
		}
		else {
			user_info *check_usr=NULL;
			check_usr=(user_info *) malloc(sizeof(user_info *));
            list_find_socket(*user_list,clientSocket,&check_usr);// searches for socket
			//printf("\nadd %d\n",2);
			//print(*check_usr);
			//printf("\n:username:%s:\n",strlen((*check_usr).ui_username));
			if (isempty(*check_usr)){
				usr->ui_nick=nick;
				list_append(user_list,usr);
				//printf("\nadd %d\n",3);
			}
			else if(strlen((*check_usr).ui_username)!=0 && !is_user_registered(*usr)){
				//welcome and add nick
				(*check_usr).ui_nick=malloc(sizeof(char)*strlen(nick));
				strcpy((*check_usr).ui_nick,nick);
				list_append(user_list,&check_usr);
                
				char * buffer ;
				buffer = con_rpl_welcome( serverhost, check_usr );
				send_rpl( clientSocket, buffer );
				//printf("\nadd %d\n",4);
			}
			else{
				(*check_usr).ui_nick=malloc(sizeof(char)*strlen(nick));
				strcpy((*check_usr).ui_nick,nick);
				list_append(user_list,&check_usr);
				//printf("\nadd %d\n",5);
			}
		}
	}
	//printlist(*user_list);
}

void add_user_by_uname(char* username,char* full_username,user_info* usr, list_t* user_list,char* serverhost){
	if( list_size( user_list)==0){
        list_init( user_list);    // initialize the list
	}
	else {
		user_info *check_usr=NULL;
		int clientSocket=usr->ui_socket;
		check_usr=(user_info *) malloc(sizeof(user_info *));
		list_find_socket(*user_list,clientSocket,&check_usr);// searches for socket
		//printf("\nadd %d\n",2);
		//print(*check_usr);
		if (isempty(*check_usr)){
			usr->ui_username=username;
			if(strlen(full_username)!=0)
				usr->ui_fullname=full_username;
			list_append(user_list,usr);
		}
		else if(strlen((*check_usr).ui_nick)!=0 && !is_user_registered(*usr) ){
			(*check_usr).ui_username=malloc(sizeof(char)*strlen(username));
			strcpy((*check_usr).ui_username,username);
			if(strlen(full_username)!=0)
				check_usr->ui_fullname=full_username;
			list_append(user_list,&check_usr);
			//New username is added
			char *buffer;
			buffer = con_rpl_welcome( serverhost, *usr );
			strcpy(buffer,"Welcome");
            send_rpl( clientSocket, buffer );
		}
		else{
			(*check_usr).ui_username=malloc(sizeof(char)*strlen(username));
			strcpy((*check_usr).ui_username,username);
			if(strlen(full_username)!=0)
				usr->ui_fullname=full_username;
			list_append(user_list,&check_usr);
		}
	}
	//printlist(*user_list);
}

void list_find_socket(list_t user_list,int socket,user_info** p_usr){
	user_info usr;
	*p_usr=(user_info *) malloc(sizeof(user_list));
	**p_usr=init_user();
	int i;
	for(i=0;i<list_size(&user_list);i++){
		usr = *(user_info *)list_get_at( &user_list, i);
		if(usr.ui_socket==socket){
			
			*p_usr=(user_info *)list_get_at( &user_list, i);
			//printf("Nikita");
			//print(**p_usr);
			break;
		}
	}
}

bool is_nick_present(list_t user_list,char* nick){
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

