#ifndef __MY_CLIENT_H
#define __MY_CLIENT_H


int connect_serv(const char const *ip,short port);
int delStdinInput(int sockfd,char *cmd);
int readClientRecv(int sockfd);
int dealClientRecv(unsigned short cmd,unsigned int packet_len,char *buf);



#endif // !__MY_CLIENT_H