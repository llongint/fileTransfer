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
#include <sys/wait.h>

#include "my_socket.h"
#include "my_io.h"

int main(int argc,char *argv[])
{
    if(argc<2){
        printf("./server [port]\n");
        exit(0);
    }

    /* 初始化工作目录 */
    if( file_init(g_server_work_path,serv_public_key,serv_private_key,&g_servUserdata,serv_userDataFile) == -1){
        return -1; 
    }
    pid_t pid;

    /* 1.创建TCP服务 */
    int serv_fd=creatTcpServer(atoi(argv[1]));
    if(serv_fd==-1){
        perror("creatTcpServer failed");
        exit(0);
    }

    /* 2.监听套接字 */
    int err=listen(serv_fd,5);
    if(err==-1){
        perror("listen");
        exit(0);
    }

    struct sockaddr_in cliAddr;
    bzero(&cliAddr,sizeof(cliAddr));
    socklen_t cliAddrlen=sizeof(cliAddr);

    while(1){

    /* 3.接受连接 */
        int cli_fd=accept(serv_fd,(struct sockaddr *)&cliAddr,&cliAddrlen);
        print("connection from %s,port:%d\n",inet_ntoa(((struct sockaddr_in)cliAddr).sin_addr),ntohs(((struct sockaddr_in)cliAddr).sin_port));
        
        pid=fork();
        if(pid == 0){       //子程序处理新的连接
            client_handle(cli_fd);
        }else if(pid>0){
            if(cli_fd >2){
                close(cli_fd);  //父进程关闭引用描述符
            }
        }else{
            perror("fork");
            exit(0);
        }
    }
    return 0;
}