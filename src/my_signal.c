#include "my_signal.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "my_socket.h"
#include "my_signal.h"

/**
 * @brief  子程序退出处理函数，防止出现僵尸进程
 * @note   
 * @param  signo: 
 * @retval None
 */
void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    pid=wait(&stat);

    print("child %d terminated\n",pid);
}
