/*
 * @Author: hzq 
 * @Date: 2018-09-11 21:49:55 
 * @Last Modified by: hzq
 * @Last Modified time: 2018-09-12 23:07:00
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "my_login.h"
#include "my_io.h"
#include "format.h"
#include "my_socket.h"

int select_read = 1;

/** 
 * @brief  
 * @note   
 * @param  sock: 
 * @param  *cmd:    用户名+密码+验证码(暂时先用固定的)，换行符分隔，最长32个字符
 *      @example    hzq\n!$@*%&\n142857\n\0
 * @retval None
 */
void sendRegisterCmd(int sock, char *cmd)
{
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);

    select_read = 0;

    printf("user name: ");
    fgets(cmd,max_string_len,stdin);
    printf("paswd: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);
    printf("identifying code: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);
    

    select_read = 1;

    head_package(send_cmd,e_msgRegist,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符
}
/**
 * @brief  发送ls命令
 * @note   就是发送session
 * @param  sockfd: 
 * @param  *cmd: 
 * @retval None
 */
void send_cmd_ls(int sockfd, char *cmd)
{
	char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);

    head_package(send_cmd,e_msgLs,max_string_len);        //加上结束符
    memcpy(send_cmd+Msg.m_msgLen,g_cliUserdata->m_session,max_string_len);
    Write(sockfd, send_cmd, Msg.m_msgLen+max_string_len);//发送结束符
}
void send_cmd_put(int sockfd, char *cmd)
{
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);

    /* 消息前加上32字节的session */
    memcpy(send_cmd+Msg.m_msgLen,g_cliUserdata->m_session,max_string_len);
    snprintf(send_cmd+Msg.m_msgLen+max_string_len,maxMessageSize-Msg.m_msgLen-max_string_len,"%s",cmd);
    head_package(send_cmd,e_msgSendFile,strlen(cmd)+1+max_string_len);
    Write(sockfd,send_cmd,strlen(cmd)+1+Msg.m_msgLen+max_string_len);
    //printf("send put :%s\n",cmd);
}

/**
 * @brief  发送获取文件命令
 * @note   
 * @param  sock: 
 * @param  *fileName: 
 * @retval None
 */
void send_cmd_get(int sock, char *fileName)
{
    char *filename = correctName(fileName);
    if(g_cliUserdata->m_filefd > 2){
        close(g_cliUserdata->m_filefd);
        g_cliUserdata->m_filefd = -1;
    }
    
    g_cliUserdata->m_filefd=-1;

    char str[max_string_len];
    strcpy(str,filename);
    while(1){
        print("filename:%s\n",str);
        g_cliUserdata->m_filefd=open(str, O_WRONLY|O_CREAT|O_EXCL,FILE_MODE);
        if(g_cliUserdata->m_filefd == -1){//打开失败
            if(errno==EEXIST){
                create_rand_num(4,str);
                printf("file exist,name will be randed as %s\n",str);
            }else{
                perror("open failed");
                return ;
            }
        }else{
            break;
        }
    }
    /* 消息前加上32字节的session */
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);
    
    memcpy(send_cmd+Msg.m_msgLen,g_cliUserdata->m_session,max_string_len);
    snprintf(send_cmd+Msg.m_msgLen+max_string_len,maxMessageSize-Msg.m_msgLen-max_string_len,"%s",filename);
    head_package(send_cmd,e_msgGet,strlen(filename)+1+max_string_len);
    Write(sock,send_cmd,strlen(filename)+1+Msg.m_msgLen+max_string_len);
    //printf("send get :%s\n",filename);
}
/**
 * @brief  发送登录命令
 * @note   举例：       hzq\n123\n\0
 * @param  sock: 
 * @param  *cmd: 
 * @retval None
 */
void sendLoginCmd(int sock, char *cmd)
{
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);
    select_read = 0;

    printf("user name: ");
    fgets(cmd,max_string_len,stdin);
    printf("paswd: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);

    select_read = 1;

    printf("waiting server return for Login:\n");
    head_package(send_cmd,e_msgLogin,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);      //发送结束符
}
/**
 * @brief  验证登录信息
 * @note   hzq\n123\n\0
 * @param  *buf: 
 * @retval 
 */
int user_confirmation(int sockfd,struct User **g_userdata,char *buf)
{
    print("start of user_confirmation\n");
    char path[maxMessageSize];
    assert(g_userdata != NULL);
    char *p1 = NULL,*p2 = NULL;
    struct User *p = *g_userdata;
    p1 = strstr((const char *)buf,(const char *)"\n");
    p2 = strstr((const char *)p1+1,  (const char *)"\n");
    if(p1==NULL || p2==NULL || p1-buf >=32 || p2-p1 >= 32){
        printf("login format error\n");
        return e_formatErr;
    }
    *p1 = '\0';
    *p2 = '\0';
    print("buf = %s\n",buf);
    print("p1+1 = %s\n",p1+1);
    while(p != NULL){
        if(strlen(p->m_name) == strlen(buf) &&strlen(p->m_passwd) ==strlen(p1+1)&& memcmp(p->m_name,buf,strlen(buf)) == 0 && memcmp(p->m_passwd,p1+1,strlen(p1+1)) == 0){
            p->m_sockfd = sockfd;
            int err = create_rand_string(31,p->m_session);      
            assert(err == 0);
            err = send_session(sockfd,p->m_name,p->m_identification,p->m_session);
            assert(err == 0);
            snprintf(path,maxMessageSize,"%s/%s",g_server_work_path,p->m_identification);
            err = change_dir(path);
            return err;
        }
        p=p->next;
    }
    print("end of %s\n",__FUNCTION__);
    return e_UserOrPasswdWrong;
}
/**
 * @brief  发送姓名标识号和session
 * @note   hzq\n88652751513\n[˘'e종*
 * @param  sockfd: 
 * @param  *m_identification: 
 * @param  *m_session: 
 * @retval 
 */
int send_session(int sockfd,char *m_name,char *m_identification,char *m_session)
{
    printf("start of send_session\n");
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);

    //head_package(send_cmd,e_msgSession,max_string_len);        //加上结束符
    int num = snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize,"%s\n%s\n",m_name,m_identification);
    head_package(send_cmd,e_msgSession,num+max_string_len); 
    memcpy((char *)send_cmd+Msg.m_msgLen+num,m_session,max_string_len);
    Write(sockfd, send_cmd, Msg.m_msgLen+num+max_string_len);      //发送结束符

    printf("end of send_session\n");
    return 0;
}