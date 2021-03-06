/*
 * @Author: hzq 
 * @Date: 2018-09-11 17:34:31 
 * @Last Modified by: hzq
 * @Last Modified time: 2018-09-11 22:02:29
 */
#ifndef __MY_SOCKET_H
#define __MY_SOCKET_H

//设置为1打印调试信息，0不输出调试信息
#define PRINT 0

const int maxMessageSize;

int print(char *fmt, ...);
int creatTcpServer(short port);
int client_handle(int sockfd);
void freeback2client(int sockfd,signed int err);
int delServerRecv(int sockfd,unsigned short cmd,unsigned int packet_len,char *buf);
void Write(int fd, void *ptr, size_t nbytes);
ssize_t Read(int fd, void *ptr, size_t nbytes);





#endif // !__MY_SOCKET_H