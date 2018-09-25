#ifndef __MY_IO_H
#define __MY_IO_H


#ifndef bool
#define bool char
#endif

#define	DIR_MODE    00777
#define	FILE_MODE   00777

#define max_string_len 32

struct User{
    char m_name[max_string_len];             //不要更换成员的顺序顺序 @ref read_userdata()
    char m_passwd[max_string_len];
    char m_identification[max_string_len];
    char m_session[max_string_len];
    char m_ip[max_string_len];
    short m_port;
    int m_sockfd;
    int m_filefd;
    struct User *next;
};

extern const char const* g_client_work_path;
extern const char const* g_server_work_path;

extern const char const *serv_public_key;
extern const char const *serv_private_key;
extern const char const *cli_public_key;
extern const char const *cli_private_key;

extern const char const *serv_userDataFile;
extern const char const *cli_userDataFile;

extern struct User *g_servUserdata;
extern struct User *g_cliUserdata;

int file_init(const char const *work_path,const char const *public_key,const char const *private_key,struct User **g_userdata,const char const* userDataFile);
int change_dir(const char const *work_path);
int save_userData(const char *filename,struct User **g_userdata,char *data);
int read_userdata(const char *filename,struct User **p_data);
int isIdentificationExist(struct User *g_userdata,char *string);
int create_rand_num(int num,char *string);
int create_rand_string(int num,char *string);
int isUserExist(struct User *g_userdata,char *string);
char *correctName(char *pathName);
void ls(int cli_fd,char *session);
void print_userData(struct User *g_userdata);
void free_list(struct User **p);
struct User *findUserBysockfd(int sockfd,struct User *g_userdata);
int end_of_transf(int sockfd,char *session);
int saveFileContent(int sockfd,char *session,int packet_len);
void handle_put(int sock,char *session);
void server2clientFile(int sock,char *session);

#endif // !__MY_IO_H



