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
#include "my_login.h"
#include "my_io.h"
#include "format.h"

const int maxMessageSize = 1024;

#if PRINT
static char sprint_buf[1024];
#endif
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

    //printf("local %s,port:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
    

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
            if(sockfd > 2)
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
            printf("messages is less than 6\n");
            exit(-1);
        }

        /* 4.正常读数据 */
        unsigned short cmd_num = 0;
        unsigned int packet_len = 0;
        head_analyze(buf,&cmd_num,&packet_len);
        count = Read(sockfd, buf, packet_len);
        if(count < packet_len){
            print("cmd_num = %d,count = %d\n,packet_len = %d\n",cmd_num,count,packet_len);
            printf("read failed!??\n");
            return -1;
        }
        delServerRecv(sockfd,cmd_num,packet_len,buf);
        //system("pwd");
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
    signed int err = 0;
    //printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s",buf);
            break;
        case e_msgRegist:
            //注册
            print("regist request\n");
            err = save_userData(serv_userDataFile,&g_servUserdata,buf);
            freeback2client(sockfd,err);
            break;
        case e_msgLogin:
            //登录
            //printf("login request\n");
            err = user_confirmation(sockfd,&g_servUserdata,buf);    //ref g_userdata
            //printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        case e_msgLs:
            ls(sockfd,buf);
            break;
        case e_msgEndFile:
            end_of_transf(sockfd,buf);
            break;
        case e_msgFileContent:
            saveFileContent(sockfd,buf,packet_len);
            break;
        case e_msgSendFile:
            /* 客户端的接收命令请求 */
            handle_put(sockfd,buf);
            break;
        case e_msgGet:
            server2clientFile(sockfd,buf);
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
/**
 * @brief  给客户端发送反馈信息
 * @note   
 * @param  sockfd: 连接套接字
 * @param  err: 错误吗
 * @retval None
 */
void freeback2client(int sockfd,signed int err)
{
    char send_cmd[maxMessageSize];
    char cmd[maxMessageSize];
    switch(err){
        case e_success:
            snprintf(cmd,maxMessageSize,"%s","operate success\n");
            break;
        case e_userExist:
            snprintf(cmd,maxMessageSize,"%s","the user name has already exist!\n");
            break;
        case e_wongIdent:
            snprintf(cmd,maxMessageSize,"%s","the identification is wrong\n");
            break;
        case e_formatErr:
            snprintf(cmd,maxMessageSize,"%s","format error\n");
            break;
        case e_UserOrPasswdWrong:
            snprintf(cmd,maxMessageSize,"%s","user name or password is wrong\n");
            break;
        case e_loginSuccess:
            snprintf(cmd,maxMessageSize,"%s","login success\n");
            break;
        case e_userOnline:
            snprintf(cmd,maxMessageSize,"%s","user is online,you can't login again\n");
            break;
        case e_LogoutSuccess:
            snprintf(cmd,maxMessageSize,"%s","Logout success\n");
            break;
        case e_sessionError:
            snprintf(cmd,maxMessageSize,"%s","session error\n");
            break;
        case e_fileExist:
            snprintf(cmd,maxMessageSize,"%s","file has already exist\n");
            break;
        case e_wongCharacter:
            snprintf(cmd,maxMessageSize,"%s","include illegal character\n");
            break;
        default :
            snprintf(cmd,maxMessageSize,"err = %d,%s",err,"????????????\n");
            return ;//其他错误就啥也不输出
    }
    head_package(send_cmd,e_msgDebug,strlen(cmd)+1);    //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen-1,"%s",cmd);
    write(sockfd, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符 
}