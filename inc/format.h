/*
 * @Author: hzq 
 * @Date: 2018-09-11 18:08:18 
 * @Last Modified by: hzq
 * @Last Modified time: 2018-09-11 18:09:06
 */
#ifndef __FORMAT_H
#define __FORMAT_H

/**
 * 协议总述：使用最常见的TLV格式数据
 *    T:2个字节，L:4字节，V:根据L而定
 * 类型字段 T:
 *    见MessageType
 * 注意： 
 *    数据的传输不包含'\0'
 */
#pragma pack(4)
struct MsgLen{
    unsigned int m_msgTypeLen;
    unsigned int m_msgValueLen;
    unsigned int m_msgLen;          //总长度
};
extern struct MsgLen Msg;
enum MessageType{
    e_msgRegist = 0,        //注册
    e_msgLogin,             //登录
    e_msgSession,           //session
    e_msgLogout,            //注销
    e_msgAddFriend,         //添加好友，要求知道对方的姓名和QQ号
    e_msgDebug,             //debug阶段用的一个消息类型
    e_msgQuit,              //退出登录
    e_msgChart,             //聊天消息
    e_msgLs,                //Ls命令
    e_msgGet,               //文件请求命令
    e_msgFileHead,          //文件头信息
    e_msgFileContent,       //文件内容
    e_msgEndFile,           //文件传输完成
    e_msgSendFile,          //文件上传请求
    e_msgFileLegal,         //文件名合法，可以上传文件
    e_msgFileNonExist,      //文件不存在
    e_msgPwd,               //PWD命令
    e_msgCd                 //cd命令，改变工作路径
}messageType;

enum ErrNum{
    e_success = 1,          //操作成功
    e_userExist,            //用户名已存在
    e_wongIdent,            //错误验证码
    e_formatErr,            //格式错误
    e_UserOrPasswdWrong,    //用户名或密码错误
    e_loginSuccess,         //登录成功
    e_userOnline,           //用户已经在线，不能重复登录
    e_LogoutSuccess,
    e_
}errnum;

int IsBigEndian();
int uintToString(char *str,unsigned int num);
int stringToUint(char *str,unsigned int *num);
void  head_package(char *send_cmd,unsigned short cmd_num,unsigned int packet_len);
void head_analyze(char *p,unsigned short *cmd_num,unsigned int *packet_len);

#endif