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

#include "my_client.h"
#include "my_socket.h"
#include "my_signal.h"
#include "format.h"

/** 
 * @brief  与服务器建立TCP连接
 * @note   
 * @param  *ip: 
 * @param  port: 
 * @retval 成功返回套接字描述符.失败返回-1
 */
int connect_serv(const char const *ip,short port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd != -1);

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    bzero(&clientAddr,addrlen);
    clientAddr.sin_family=AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = 0;
    int err = bind(sockfd,(struct sockaddr*)&clientAddr,addrlen);//不绑定也行的
    assert(err != -1);

    clientAddr.sin_addr.s_addr = inet_addr(ip);
    clientAddr.sin_port = htons(port);
    err = connect(sockfd,(struct sockaddr*)&clientAddr,addrlen);
    assert(err != -1);

    return sockfd;
}

/** 
 * @brief  处理客户端标准输入
 * @note   
 * @param  sock: 套接字描述符
 * @param  *cmd: 命令
 * @retval 成功返回0,失败返回一个负数
 */
int delStdinInput(int sockfd,char *cmd)
{
    if(strncmp(cmd, "regist", strlen("regist")) == 0){
		//sendRegisterCmd(sock, cmd);
	}else{
        //没有检查到关键词
        char send_cmd[maxMessageSize];
        head_package(send_cmd,e_msgDebug,strlen(cmd)+1);        //加上结束符
        snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen-1,"%s",cmd);
        write(sockfd, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符
    }
    return 0;
}
/** 
 * @brief  处理客户端接收的套接字数据
 * @note   
 * @param  sock: 
 * @retval 成功返回0,失败返回一个负数
 */
int readClientRecv(int sockfd)
{
    assert(sockfd != -1);

    char buf[maxMessageSize];
    int count=0;
    bzero(buf,maxMessageSize);

    /* 1.读取出错 */
    if ( (count = read(sockfd, buf, Msg.m_msgLen)) < 0) {
        close(sockfd);
        perror("read sockfd");
        exit(0);
        return -2;
    }

    /* 2.服务器发送FIN */
    else if (count == 0){
        printf("server closed!\n");
        exit(0);
        return -3;
    }
    /* 3.没有读到6个字节 */
    else if(count < 6){
        exit(0);
        return -1;
    }

    /* 4.正常读数据 */
    unsigned short cmd_num = 0;
    unsigned int packet_len = 0;
    head_analyze(buf,&cmd_num,&packet_len);
    count = Read(sockfd, buf, packet_len);
    if(count < packet_len){
        print("count = %d,packet_len = %d\n",count,packet_len);
        printf("read failed!\n");
        exit(0);
        return -1;
    }
    int err = dealClientRecv(cmd_num,packet_len,buf);
    
    return err;
}
/**
 * @brief  处理客户端接收的数据
 * @note   
 * @param  cmd: 
 * @param  packet_len: 
 * @param  *buf: 
 * @retval 
 */
int dealClientRecv(unsigned short cmd,unsigned int packet_len,char *buf)
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