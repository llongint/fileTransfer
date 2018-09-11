#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdarg.h>

#include "my_socket.h"
#include "my_signal.h"
#include "format.h"

const int maxMessageSize = 1024;

static char sprint_buf[1024];
int print(char *fmt,...)
{
    int n = 0;
#if PRINT
    va_list args;
    va_start(args,fmt);
    n = vsprintf(sprint_buf,fmt,args);
    va_end(args);
    write(fileno(stdout),sprint_buf,n);

#endif
    return n;
}


/**
 * @brief  创建TCP服务，默认本地ip
 * @note   
 * @param  port: 端口号
 * @retval 
 */
int creatTcpServer(short port)
{
    int sock_fd=-1;

    /* 1.创建套接字 */
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    assert(sock_fd!=-1);

    /* 2.设置地址 */
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);

    /* 3.绑定地址 */
    int err = bind(sock_fd,(struct sockaddr *)&addr,sizeof(addr));
    if(err==-1){
        perror("bind ");
        exit(0);
    }

    /* 4.启动监听 */
    err = listen(sock_fd,5);
    if(err==-1){
        perror("listen ");
        exit(0);
    }

    /* 5.设置子程序退出信号处理函数 */
    signal(SIGCHLD,sig_chld);

    return sock_fd;
}
/**
 * @brief  服务端处理用户端的请求
 * @note   
 * @param  cli_fd: 
 * @retval None
 */
int client_handle(int sockfd)
{

    char buf[maxMessageSize];
    bzero(buf,maxMessageSize);
    int count=0;
    while(1){
        /* 1.对端发送FIN后，还向这个套接字写会收到 RST */
        if ( (count = read(sockfd, buf, Msg.m_msgLen)) < 0) {
            perror("read sockfd");
            close(sockfd);    
            exit(-2);
        }
        /* 2.对方发送了FIN,服务器读会返回0，应答后处于CLOSE_WAIT状态 */
        else if (count == 0){
            printf("a user closed\n");
            close(sockfd);
            exit(-3);
        }
        /* 3.没有读到6个字节 */
        else if(count < 6){
            exit(-1);
        }

        /* 4.正常读数据 */
        unsigned short cmd_num = 0;
        unsigned int packet_len = 0;
        head_analyze(buf,&cmd_num,&packet_len);
        count = Read(sockfd, buf, packet_len);
        if(count < packet_len){
            printf("read failed!\n");
            return -1;
        }
        delServerRecv(sockfd,cmd_num,packet_len,buf);
    }

    return 0;
}
/** 
 * @brief  处理服务端接收的套接字数据
 * @note   
 * @param  cmd: 
 * @param  packet_len: 
 * @param  *buf: 
 * @retval 
 */
int delServerRecv(int sockfd,unsigned short cmd,unsigned int packet_len,char *buf)
{
    //signed int err = 0;
    printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s",buf);
            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
void Write(int fd, void *ptr, size_t nbytes)
{
	int r=write(fd, ptr, nbytes);
    if ( r!= nbytes){
		printf("write error: %d %d\n",r,(int)nbytes);
        exit(-1);
    }
}
ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1){
		perror("read error");
    }
	return(n);
}