#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <dirent.h>

#include "my_io.h"
#include "format.h"
#include "rsa.h"
#include "my_socket.h"

/* 注册时需要填写的验证码，暂时写死 */
static const char const *g_identifying_code = "076923";

/* 工作目录 */
const char const *g_client_work_path = "/home/fileTransfer.client";
const char const *g_server_work_path = "/home/fileTransfer.server";

/* 秘钥文件名 */
const char const *serv_public_key = "serv.rsa.public";
const char const *serv_private_key = "serv.rsa.private";
const char const *cli_public_key = "cli.rsa.public";
const char const *cli_private_key = "cli.rsa.private";

/* 配置文件名 */
const char const *serv_userDataFile = "serv.user.data";
const char const *cli_userDataFile = "cli.user.data";

/* 用户信息结构体头指针 */
struct User *g_servUserdata = NULL;
struct User *g_cliUserdata = NULL;



/**
 * @brief  改变工作路径,并在当前目录生成秘钥文件
 * @note   
 * @param  *work_path: 工作路径
 * @param  *public_key: 公钥文件名
 * @param  *private_key: 私钥文件名
 * @param  **g_userdata: 用户信息全局结构体
 * @param  userDataFile: 用户配置信息保存的文件
 * @retval 
 */
int file_init(const char const *work_path,const char const *public_key,const char const *private_key,
              struct User **g_userdata,const char const* userDataFile)
{
    int err = mkdir(work_path,DIR_MODE);
    if(err == -1){
        if(errno == EEXIST){
            ;//已经存在就不用做什么
        }
        //如果路径不存在,就是说没有/home目录
        else if(errno == ENOENT){
            mkdir("/home/",DIR_MODE);
            mkdir(work_path,DIR_MODE);
        }else{
            perror("mkdir");
            return -1;
        }
    }
    print("created work path\n");

    err = chdir(work_path);
    if(err == -1){
        perror("chdir");
    }
    print("work path changed:\n\t\t");
    system("pwd");

    int fd1 = open(public_key,O_WRONLY | O_EXCL | O_CREAT | FILE_MODE);
    int fd2 = open(private_key,O_WRONLY | O_EXCL | O_CREAT | FILE_MODE);
    if(fd1 == -1 && fd2 == -1){
        if(errno == EEXIST){                //如果两个文件都已经存在
            print("file has created\n");    //说明秘钥文件已经生成
        }else{ 
            print("function:%s,line:%d\n",__FUNCTION__,__LINE__);   
            perror("open");                          //如果碰到了其他问题
            return -1;                      //并退出
        }
    }else{                                  //如果缺少秘钥文件
        print("%s and %s is not created,now is trying to create it\n",public_key,private_key);
        err = create_key(public_key,private_key);     //则重新创建
        assert(err != -1);
    }

    print("key file created\n");
    
    err = read_userdata(userDataFile,g_userdata);
    print("read user data success\n");

    return err;
}
int change_dir(const char const *work_path)
{
    int err = mkdir(work_path,DIR_MODE);
    if(err == -1){
        if(errno == EEXIST){
            ;//已经存在就不用做什么
        }
        //如果路径不存在,就是说没有/home目录
        else if(errno == ENOENT){
            mkdir("/home/",DIR_MODE);
            mkdir(work_path,DIR_MODE);
        }else{
            perror("mkdir");
            return -1;
        }
    }
    print("created work path\n");

    err = chdir(work_path);
    if(err == -1){
        perror("chdir");
    }
    print("work path changed:\n\t\t");
    system("pwd");
    
    return err;
}

/** 
 * @brief  读用户的注册文件，并用保存保存到结构体 struct user
 * @note   
 * @param  *filename: 文件名
 * @retval 成功返回0,失败返回负数
 */
int read_userdata(const char *filename,struct User **p_data)
{
    print("now try to open : %s\n",filename);
    /* 1.检查配置文件 */
    int fd_userData = open(filename,O_WRONLY | O_EXCL | O_CREAT ,FILE_MODE);
    if(fd_userData != -1){
        print("creat file \"%s\" success\n",filename);
        char buf[maxMessageSize];
        /* 用户名+密码+识别号 */
        snprintf(buf,maxMessageSize,"root\037142857Hzq076923\037142857\n");
        Write(fd_userData,buf,strlen(buf));
        if(fd_userData > 2){
            close(fd_userData);
            fd_userData = -1;
        }
    }else{
        perror("creat user data ");
    }
    fd_userData = open(filename,O_RDONLY);
    if(fd_userData == -1){
        perror("open user data for read");
        exit(0);
    }
    
    printf("open \"%s\" success,fd = %d\n",filename,fd_userData);
    int i = 0;
    int index = 0;  //记录当前读到哪个字段
    char ch = 0;
    struct User *p = NULL,*p1=NULL;

    free_list(p_data);//先释放这个指针
    while(1){
        p = (struct User *)calloc(1,sizeof(struct User));
        //bzero(p,sizeof(struct User));
        assert(p != NULL);

        while(Read(fd_userData,&ch,1) > 0){

            /* 1.长度超过了限制 */
            if(i >= max_string_len){
                printf("file may have been damaged\n");
                free(p);
                p = NULL;
                i = 0;
                index = 0;
                break;
            }

            /* 2.遇到换行符的时候判断时候接受完成 */
            if(ch == '\n'){
                if(index != 2){
                    printf("file may have been damaged\n");
                    free(p);
                    p = NULL;
                }
                p->m_identification[i] = 0;//添加结束符
                i = 0;
                index = 0;
                break;
            }
            /* 3.碰到分隔符 */
            else if(ch == '\037'){
                ((char *)p)[index*max_string_len + i] = '\0';    //添加结束符
                index++;
                i = 0;
            }
            /* 4.正常字符 */
            else{
                ((char *)p)[index*max_string_len + i++] = ch;
            }
        }
        /* 1.读到文件尾或者遇到错误 */
        if(((char *)p)[0] == 0){
            break;
        }
        /* 正常读完一个用户的数据 */
        else{
            if(*p_data == NULL){
                *p_data = p;
                p1 = p;
            }else{
                p1->next = p;
                p1 = p1->next;
            }
        }
    }
    printf("read over\n");
    print_userData(*p_data);
    if(fd_userData > 2){
        close(fd_userData);
        fd_userData = -1;
    }
    return 0;
}
/** 
 * @brief  保存用户信息,保存到链表和文件
 * @note   暂时用文件维护，后面再用数据库好了
 * @param  *data: 格式如:   hzq\n!$@*%&\n142857\n\0
 * @param  *filename: 
 * @retval 
 */
int save_userData(const char *filename,struct User **g_userdata,char *data)
{
    assert(g_userdata != NULL);

    char *p1 = NULL,*p2 = NULL,*p3 = NULL;
    p1 = strstr((const char *)data,(const char *)"\n");
    p2 = strstr((const char *)p1+1,  (const char *)"\n");
    p3 = strstr((const char *)p2+1,  (const char *)"\n");
    if(p1==NULL || p2==NULL || p3 == NULL || p1-data >=32 || p2-p1 >= 32){
        printf("data format error\n");
        return -1;
    }
    *p1 = '\037';
    *p2 = '\037';
    *p3 = '\00';

    if(memcmp(p2+1,g_identifying_code,strlen(g_identifying_code)) != 0){
        print("%s\n",p2+1);
        print("%s\n",g_identifying_code);
        print("identification error\n");
        return e_wongIdent;
    }
    *(p2+1) = '\0';
    char userdata[maxMessageSize];
    char ident_string[max_string_len];
    create_rand_num(11,ident_string);
    snprintf(userdata,maxMessageSize,"%s%s\n",data,ident_string);

    printf("analyse success\n");
    *p1 = '\0';
    *p2 = '\0';
    if(isUserExist(*g_userdata,data)){
        printf("user exist\n");
        return e_userExist;
    }

    int fd = open(filename,O_WRONLY|O_APPEND);
    assert(fd != -1);
    printf("try to write to user data file\n");
    Write(fd,userdata,strlen(userdata));
    if(fd > 2){
        close(fd);
        fd = -1;
    }
 
    struct User *p4=*g_userdata;
    struct User *p =calloc(1,sizeof(struct User));
    memcpy(p->m_name,data,strlen(data)+1);
    memcpy(p->m_passwd,p1+1,strlen(p1+1)+1);
    memcpy(p->m_identification,ident_string,strlen(ident_string)+1);
    while(p4->next !=NULL)
        p4=p4->next;
    p4->next =p;

    print_userData(*g_userdata);

    return 0;
}

/**
 * @brief  释放节点数据
 * @note   
 * @param  **p: 
 * @retval None
 */
void free_list(struct User **p)
{
    struct User *p1=*p;
    while(p1){
        p1=p1->next;
        free(*p);
        *p=p1;
    }
    *p=NULL;
}
void print_userData(struct User *g_userdata)
{
    struct User *p = g_userdata;
    printf("------------------------user information-----------------------------\n");
    while(p != NULL){
        printf("user name:   %s\n",p->m_name);
        printf("user passwd: %s\n",p->m_passwd);
        printf("user ident..:%s\n",p->m_identification);
        printf("user sess..: %s\n",p->m_session);
        printf("user ip:     %s\n",p->m_ip);
        printf("user port:   %d\n",p->m_port);
        printf("user sockfd  %d\n",p->m_sockfd);
        printf("------------------------------------------\n");
        p=p->next;
    }
    printf("------------------------------------------end of the information-----\n");
}
/** 
 * @brief  生成一个随机数字串,不会和已有的QQ号重复
 * @note   string的长度必须大于num,因为要加上结束符
 * @param  num: 
 * @param  *string: 
 * @retval 成功返回0,失败返回-1
 */
int create_rand_num(int num,char *string)
{
    int i = 0,count = 0;
    assert(num > 0);
    while(count < 20){
        int err = create_rand_string(num,string);
        assert(err != -1);
        
        for(i=0;i<num;i++){
            string[i] = (abs(string[i]) % 9) + '0';
        }
        string[num] = '\0';
        if(!isIdentificationExist(g_servUserdata,string)){
            break;
        }   
        count++;
        if(count > 20){
            printf("create rand num failed\n");
            return -1;
        }
    }
    return 0;

}
int create_rand_string(int num,char *string)
{
    assert(num > 0);
    assert(string != NULL);
    int fd_rand = open("/dev/urandom",O_RDONLY);
    if(fd_rand == -1){
        perror("open /dev/urandom");
        return -1;
    }
    int i = 0;
    while(i<num){
        i += Read(fd_rand,string+i,num-i);
    }
    if(fd_rand > 2){
        close(fd_rand);
        fd_rand = -1;
    }
    
    return 0;
}
int isIdentificationExist(struct User *g_userdata,char *string)
{
    struct User *p = g_userdata;
    while(p != NULL){
        if(strlen(string) != strlen(p->m_identification)){
            p=p->next;
            continue;
        }
        if(memcmp(string,p->m_identification,strlen(string)) == 0){
            return 1;
        }
        p=p->next;
    }
    return 0;
}
int isUserExist(struct User *g_userdata,char *string)
{
    struct User *p = g_userdata;
    while(p != NULL){
        if(strlen(string) != strlen(p->m_name)){
            p=p->next;
            continue;
        }
        if(memcmp(string,p->m_name,strlen(string)) == 0){
            return 1;
        }
        p=p->next;
    }
    return 0;
}

int end_of_transf(int sockfd,char *session)
{
    /* 1.验证session */
    struct User *p=findUserBysockfd(sockfd,g_servUserdata);
    if(p==NULL || memcmp(p->m_session,session,max_string_len)!=0){
        print("recive :%s\n",session);
        print("local  :%s\n",p->m_session);
        freeback2client(sockfd,e_sessionError);
        return e_sessionError;
    }
    if(p->m_filefd >2){
        close(p->m_filefd);
        p->m_filefd = -1;
    }

    print("recive finished\n");
    return 0;
}
int saveFileContent(int sockfd,char *session,int packet_len)
{
    /* 1.验证session */
    struct User *p=findUserBysockfd(sockfd,g_servUserdata);
    if(p==NULL || memcmp(p->m_session,session,max_string_len)!=0){
        print("recive :%s\n",session);
        print("local  :%s\n",p->m_session);
        freeback2client(sockfd,e_sessionError);
        return e_sessionError;
    }
    session += max_string_len;
    lseek(p->m_filefd,0,SEEK_END);
    Write(p->m_filefd,session,packet_len-max_string_len);

#ifndef NOPRINT
    unsigned short len=0;
    unsigned int bytes=0;
    head_analyze(session,&len,&bytes);
    print("len = %d,bytes = %d\n",len,bytes);
#endif // !NOPRINT

    return 0;
}
/**
 * @brief  读取目录下的内容，并发送到对端
 * @note   
 * @param  cli_fd: 
 * @param  *dirName: session
 * @retval None
 */
void ls(int cli_fd,char *session)
{
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);

    /* 1.验证session */
    struct User *p=findUserBysockfd(cli_fd,g_servUserdata);
    if(p==NULL || memcmp(p->m_session,session,max_string_len)!=0){
        print("recive :%s\n",session);
        print("local  :%s\n",p->m_session);
        freeback2client(cli_fd,e_sessionError);
        return ;
    }

    DIR *d1 = opendir("./");
	if(d1 == NULL){
		perror("DIR open error");
		return ;
	}
    struct dirent *d1_read=NULL;
    while(1){
        d1_read=readdir(d1);
		if(d1_read==NULL){
			//perror("read_dir");
			break;
		}
        bzero(send_cmd,maxMessageSize);
        snprintf(send_cmd+Msg.m_msgLen,maxMessageSize-Msg.m_msgLen,"%s",d1_read->d_name);
        //printf("type:%d,len:%d\n",0x0001,strlen(send_cmd+6)+1);
        head_package(send_cmd,e_msgDebug,strlen(d1_read->d_name)+1);
        Write(cli_fd,send_cmd,strlen(d1_read->d_name)+1+Msg.m_msgLen);
        puts(d1_read->d_name);
        //printf("name:%s\n",send_cmd+6);
    }
}
/**
 * @brief  处理用户的文件上传请求
 * @note   
 * @param  sock: 
 * @param  *fileName: session+文件名
 * @retval None
 */
void handle_put(int sock,char *session)
{
    /* 1.检查session */
    struct User *p=findUserBysockfd(sock,g_servUserdata);
    if(p==NULL || memcmp(p->m_session,session,max_string_len)!=0){
        print("recive :%s\n",session);
        print("local  :%s\n",p->m_session);
        freeback2client(sock,e_sessionError);
        return ;
    }

    session+=max_string_len;
    /* 2.检查是否存在非法字符'/',不允许访问其他目录的文件 */
    char *p3 = strstr((const char *)session,(const char *)"/");
    if(p3){
        freeback2client(sock,e_wongCharacter);
        return ;
    }

    /* 3.检查文件是否存在 */
    char *p1=correctName(session);
    print("file name:%s\n",p1);

    /* 3.1.尝试创建文件，已存在会报错 */
    p->m_filefd=open(p1, O_WRONLY|O_TRUNC|O_CREAT|O_EXCL,FILE_MODE);
    if(p->m_filefd==-1){
        if(errno==EEXIST){            
            freeback2client(sock,e_fileExist);
        }else{
            print("function:%s,line:%d\n",__FUNCTION__,__LINE__);   
            perror("open");
        }
        return ;
    }
    printf("file created,waiting to recive data...\n");

    /* 4.返回给客户端:session+可以上传的文件名 */
    session -= max_string_len;
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);
    memcpy(send_cmd+Msg.m_msgLen,session,max_string_len);
    snprintf(send_cmd+Msg.m_msgLen+max_string_len,maxMessageSize-Msg.m_msgLen,"%s",p1);
    head_package(send_cmd,e_msgfileOk,max_string_len+strlen(p1)+1);
    Write(sock,send_cmd,Msg.m_msgLen+max_string_len+strlen(p1)+1);
}
/**
 * @brief  服务端给客户端发送文件
 * @note   
 * @param  sock: 
 * @param  *session: 
 * @retval None
 */
void server2clientFile(int sock,char *session)
{

    /* 1.检查session */
    struct User *p=findUserBysockfd(sock,g_servUserdata);
    if(p==NULL || memcmp(p->m_session,session,max_string_len)!=0){
        print("recive :%s\n",session);
        print("local  :%s\n",p->m_session);
        freeback2client(sock,e_sessionError);
        return ;
    }

    print("session is right\n");
    
    session+=max_string_len;
    /* 2.检查是否存在非法字符'/',不允许访问其他目录的文件 */
    char *p3 = strstr((const char *)session,(const char *)"/");
    if(p3){
        freeback2client(sock,e_wongCharacter);
        return ;
    }

    /* 3.检查文件是否存在 */
    char *p1=correctName(session);
    print("file name:%s\n",p1);

    p->m_filefd=open(p1, O_RDONLY);
    if(p->m_filefd==-1){
        freeback2client(sock,e_fileNotExist);
        return ;
    }
    print("file open success\n");

    /* 4.读取加密文件并传输 */
    char buf[max_string_len];
    unsigned short length = 0;
    unsigned int bytes = 0;
    session -= max_string_len;
    char send_cmd[maxMessageSize];
    bzero(send_cmd,maxMessageSize);
    memcpy(send_cmd+Msg.m_msgLen,session,max_string_len);
    
    lseek(p->m_filefd,0,SEEK_SET);
    while(Read(p->m_filefd,buf,Msg.m_msgLen)==Msg.m_msgLen){
        head_analyze(buf,&length,&bytes);
        int msg_len = max_string_len+Msg.m_msgLen+length*sizeof(unsigned int);
        print("length = %d,bytes = %d,msg_len = %d\n",length,bytes,msg_len);
        /* 4.返回给客户端:文件头+session+文件+加密结构体描述+加密信息*/
        head_package(send_cmd,e_msgFileContent,msg_len);
        head_package(send_cmd+Msg.m_msgLen+max_string_len,length,bytes);
        Read(p->m_filefd,send_cmd+Msg.m_msgLen*2+max_string_len,length*sizeof(unsigned int));
        Write(sock,send_cmd,Msg.m_msgLen+msg_len);
    }

    /* 5.通知客户端发送完成 */
    head_package(send_cmd,e_msgEndFile,max_string_len);
    Write(sock,send_cmd,Msg.m_msgLen+max_string_len);
}
/**
 * @brief  去掉开头的空格，结尾的'\0'
 * @note   
 * @param  *pathName: 
 * @retval 
 */
char *correctName(char *pathName)
{
    int i=0;
    while(pathName[i]==' '){
        i++;
    }
    while(1){
        if(pathName[strlen(pathName)-1]=='\r'||pathName[strlen(pathName)-1]=='\n'||pathName[strlen(pathName)-1]==' '){
            pathName[strlen(pathName)-1]=0;
        }else{
            break;
        }
    }
   return  pathName+i;
}
struct User *findUserBysockfd(int sockfd,struct User *g_userdata)
{
    struct User *p = g_userdata;
    while(p!=NULL){
        if(p->m_sockfd == sockfd){
            return p;
        }
        p=p->next;
    }
    printf("not find such sockfd!");
    return NULL;
}