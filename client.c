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

#include "my_signal.h"
#include "my_socket.h"
#include "format.h"
#include "my_client.h"
#include "rsa.h"

int main(int argc,char *argv[])
{
    if(argc<3){
        printf("./%s [ip] [port]\n",argv[0]);
        exit(0);
    }
    int sockfd = connect_serv(argv[1],atoi(argv[2]));
    assert(sockfd != -1);

    char buf[maxMessageSize];
    int maxfdp1;
    fd_set rset;
    FD_ZERO(&rset);
    setbuf(stdout,NULL);
    while(1){
        printf("\r$ ");
        bzero(buf,maxMessageSize);
        FD_SET(fileno(stdin),&rset);
        FD_SET(sockfd,&rset);

        maxfdp1=MAX(fileno(stdin),sockfd)+1;
        select(maxfdp1,&rset,NULL,NULL,NULL);

        if(FD_ISSET(fileno(stdin),&rset)){
            fgets(buf,maxMessageSize,(stdin));
            //处理标准输入读取的命令
            delStdinInput(sockfd,buf);
        }

        if(FD_ISSET(sockfd,&rset)){
            readClientRecv(sockfd);
        }
    }

    return 0;
}
