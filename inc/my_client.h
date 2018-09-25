#ifndef __MY_CLIENT_H
#define __MY_CLIENT_H


int connect_serv(const char const *ip,short port);
int delStdinInput(int sockfd,char *cmd);
int readClientRecv(int sockfd);
int save_session(char *buf);
int dealClientRecv(unsigned short cmd,unsigned int packet_len,char *buf,int sockfd);
void sendLoginCmd(int sock, char *cmd);
void handle_get(int cli_fd,char *filename);
void send_cmd_get(int sock, char *fileName);
void decipheringFile(int cli_fd,char *filename);
void client_end_recv_file(int cli_fd,char *filename);

#endif // !__MY_CLIENT_H