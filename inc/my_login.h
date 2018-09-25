#ifndef __MY_LOGIN_H
#define __MY_LOGIN_H

#include "my_io.h"
 
extern int select_read;


void sendRegisterCmd(int sock, char *cmd);
int user_confirmation(int sockfd,struct User **g_userdata,char *buf);
int send_session(int sockfd,char *m_name,char *m_identification,char *m_session);
void send_cmd_ls(int sockfd, char *cmd);
void send_cmd_put(int sockfd, char *cmd);
#endif // !__MY_LOGIN_H