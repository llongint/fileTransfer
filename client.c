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
#include <fcntl.h>

#include "my_signal.h"
#include "my_socket.h"
#include "format.h"
#include "my_login.h"
#include "my_client.h"
#include "rsa.h"
#include "my_io.h"

int main(int argc,char *argv[])
{
    int c =0;
    if(argc<3){
        printf("./%s [ip] [port]\n",argv[0]);
        exit(0);
    }

    if( file_init(g_client_work_path,cli_public_key,cli_private_key,&g_cliUserdata,cli_userDataFile) == -1){
        return -1;
    }
    int sockfd = connect_serv(argv[1],atoi(argv[2]));
    assert(sockfd != -1);

    char buf[maxMessageSize];
    int maxfdp1;
    fd_set rset;
    FD_ZERO(&rset);
    //setbuf(stdout,NULL);
    while(1){
        printf("\r$ ");
        bzero(buf,maxMessageSize);
        FD_SET(fileno(stdin),&rset);
        FD_SET(sockfd,&rset);

        maxfdp1=MAX(fileno(stdin),sockfd)+1;
        select(maxfdp1,&rset,NULL,NULL,NULL);

        if(FD_ISSET(fileno(stdin),&rset)){
            
            if(select_read == 1){
                fgets(buf,maxMessageSize,stdin);
                if(strlen(buf) != 0){//处理标准输入读取的命令
                    delStdinInput(sockfd,buf);
                }else{
                    int flag = fcntl(fileno(stdin),F_GETFL,0);
                    fcntl(fileno(stdin),F_SETFL,flag&(~O_NONBLOCK));
                    printf("the %d time read 0 bytes????\r",c++);
                }
            }
        }

        if(FD_ISSET(sockfd,&rset)){
            readClientRecv(sockfd);
        }
    }

    return 0;
}
