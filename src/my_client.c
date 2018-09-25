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
#include "my_io.h"
#include "my_login.h"
#include "rsa.h"

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
    if(err == -1){
        printf("ip:%s,port:%d\n",ip,port);
        perror("connect server");
        exit(0);
    }

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
		sendRegisterCmd(sockfd, cmd);
	}else if(strncmp(cmd, "login", strlen("login")) == 0){
		sendLoginCmd(sockfd, cmd);
	}else if(strncmp(cmd, "ls", strlen("ls")) == 0){
		send_cmd_ls(sockfd, cmd);
	}else if(strncmp(cmd, "put", strlen("put")) == 0){
		send_cmd_put(sockfd, cmd+3);
	}else if(strncmp(cmd, "get", strlen("get")) == 0){
		send_cmd_get(sockfd, cmd+3);
	}else{
        printf("no such command\n");
        //没有检查到关键词,
        // char send_cmd[maxMessageSize];
        // head_package(send_cmd,e_msgDebug,strlen(cmd)+1);        //加上结束符
        // snprintf((char *)send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen-1,"%s",cmd);
        // write(sockfd, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符
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
        if(sockfd > 2){
            close(sockfd);
        }
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
        printf("message is less than 6\n");
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
        printf("read failed!??????\n");
        exit(0);
        return -1;
    }
    int err = dealClientRecv(cmd_num,packet_len,buf,sockfd);
    
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
int dealClientRecv(unsigned short cmd,unsigned int packet_len,char *buf,int sockfd)
{
    signed int err = 0;
    //printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s\n",buf);
            break;
        case e_msgSession:
            err = save_session(buf);
            assert(err == 0);
            print("   login success,session has saved\n");
            break;  
        case e_msgfileOk:
            handle_get(sockfd,buf);
            break;
        case e_msgFileContent:
            decipheringFile(sockfd,buf);
            break;
        case e_msgEndFile:
            client_end_recv_file(sockfd,buf);
            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
/**
 * @brief  解析字符串: hzq\n88652751513\n[˘'e종*\n
 * @note   
 * @param  *buf: 
 * @retval 
 */
int save_session(char *buf)
{
    print("begin of %s\n",__FUNCTION__);
  
    char *p1 = NULL,*p2 = NULL;
    p1 = strstr((const char *)buf,(const char *)"\n");
    p2 = strstr((const char *)p1+1,  (const char *)"\n");
    
    if(p1==NULL || p2==NULL || p1-buf >=32 || p2-p1 >= 32||p2-p1<=1){
        printf("format error\n");
        return e_formatErr;
    }
    assert(g_cliUserdata!=NULL);
    puts(p2+1);
    memcpy(g_cliUserdata->m_session,p2+1,max_string_len);
    
    print("end of %s\n",__FUNCTION__);
    return 0;
}
void client_end_recv_file(int cli_fd,char *filename)
{
    /* 1.检查session */
    if(memcmp(filename,g_cliUserdata->m_session,max_string_len) != 0){
        print("function: %s,session error??\n",__FUNCTION__);
        return ;
    }
    print("file recv transfer finished\n");
    if(g_cliUserdata->m_filefd > 2){
        close(g_cliUserdata->m_filefd);
    }
}
void decipheringFile(int cli_fd,char *filename)
{
    if(memcmp(filename,g_cliUserdata->m_session,max_string_len) != 0){
        print("function: %s,session error??\n",__FUNCTION__);
        return ;
    }
    filename += max_string_len;

    /* 1.解析加密信息结构 */
    unsigned short length = 0;
    unsigned int bytes = 0;
    head_analyze(filename,&length,&bytes);
    filename += Msg.m_msgLen;

    /* 2.尝试解密 */
    bignum *encoded = bignum_init();
    bignum *e = bignum_init(), *n = bignum_init(), *d = bignum_init();
    
    /* 2.1读取秘钥文件 */
    read_key(cli_public_key,e,n);
    read_key(cli_private_key,d,n);

    encoded->length = length;
    memcpy((char *)(encoded->data),filename,length*(sizeof(unsigned int)));
    int *decoded = decodeMessage(1, bytes, encoded, d, n);


    //free(encoded);
	free(decoded);
    bignum_deinit(encoded);
    bignum_deinit(n);
	bignum_deinit(e);
	bignum_deinit(d);
}
/**
 * @brief  打开文件发，加密后
 * @note   
 * @param  cli_fd: 
 * @param  *filename: 
 * @retval None
 */
void handle_get(int cli_fd,char *filename)
{
    FILE *fd = NULL;
    char send_cmd[maxMessageSize];

    /* 1.检查session */
    if(memcmp(filename,g_cliUserdata->m_session,max_string_len) != 0){
        print("function: %s,session error??\n",__FUNCTION__);
        return ;
    }

    filename += max_string_len;
    char *fileName=correctName(filename);

    /* 2.尝试打开文件 */
    printf("request file: %s\n",fileName);
    fd = fopen(fileName, "r");
	if(fd == NULL) {
		print("function:%s,line:%d\n",__FUNCTION__,__LINE__);   
        perror("open");
        bzero(send_cmd,maxMessageSize);
        head_package(send_cmd,e_msgFileNonExist,0);
        Write(cli_fd,send_cmd,Msg.m_msgLen);
        return ;
	}

    /* 3.对文件进行加密 */
    int  i,len,bytes;
    bignum *encoded;
    //int *decoded;
    char *buffer;
    bignum *bbytes = bignum_init(), *shift = bignum_init();
    bignum *e = bignum_init(), *n = bignum_init(), *d = bignum_init();
    
    /* 3.1读取秘钥文件 */
    read_key(cli_public_key,e,n);
    read_key(cli_private_key,d,n);

    /* 计算在一个加密过程中能够加密的最大数字 */
    bytes = -1;
	bignum_fromint(shift, 1 << 7); /* 7 bits per char */
	bignum_fromint(bbytes, 1);
	while(bignum_less(bbytes, n)) {
        /* Shift by one byte, NB: we use bitmask representative so this can actually be a shift... */
		bignum_imultiply(bbytes, shift);
		bytes++;
	}

    /* 3.读取全部文件，并加密 */
    len = readFile(fd, &buffer, bytes);
    printf("size:%d ,try to encoding...\n",len);
    encoded = encodeMessage(len, bytes, buffer, e, n);
    printf("\n\nEncoding finished successfully ... ");
    
    print("len = %d,bytes = %d\n",len,bytes);

    /* 4.数据格式：6字节数据头，32字节session,6字节关于加密结构的描述，再接加密数据 */
    bzero(send_cmd,maxMessageSize);
    memcpy(send_cmd+Msg.m_msgLen,g_cliUserdata->m_session,max_string_len);
    for(i = 0; i < len/bytes; i++){
        int msg_len = Msg.m_msgLen+max_string_len+(encoded[i].length)*sizeof(unsigned int);
        print("i = %d,encoded[i].length = %d,bytes = %d,msg_len = %d\n",i,encoded[i].length,bytes,msg_len);
        head_package(send_cmd,e_msgFileContent,msg_len);
        head_package(send_cmd+Msg.m_msgLen+max_string_len,encoded[i].length,bytes);
        memcpy(send_cmd+Msg.m_msgLen*2+max_string_len,(char *)encoded[i].data,encoded[i].length*sizeof(unsigned int));
        Write(cli_fd,send_cmd,Msg.m_msgLen+msg_len);
    }
    printf("send finished...\n");
    head_package(send_cmd,e_msgEndFile,max_string_len);
    Write(cli_fd,send_cmd,Msg.m_msgLen+max_string_len);
    if(fd !=NULL){
        fclose(fd);
        fd = NULL;
    }
    for(i = 0; i < len/bytes; i++){
        free(encoded[i].data);
    }

    free(encoded);
	//free(decoded);
	free(buffer);
	bignum_deinit(e);
	bignum_deinit(n);
	bignum_deinit(d);
    bignum_deinit(bbytes);
	bignum_deinit(shift);
}