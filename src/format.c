
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "format.h"

/* 消息头长度结构体，记录各字段长度 */
struct MsgLen Msg = {sizeof(short),sizeof(unsigned int),sizeof(short)+sizeof(unsigned int)};

/** 
 * @brief  判断主机字节序
 * @note   
 * @retval 大端返回1,小端返回0
 */
int IsBigEndian()
{
    union temp{
        short int a;
        char b;
    }temp;
    temp.a = 0x1234;
    if( temp.b == 0x12 ){   //低字节存的是数据的高字节数据
        return 1;           //是大端模式
    }else{
        return 0;           //是小端模式
    }
}

/** 
 * @brief  格式化发送头
 * @note   暂时没有考虑大小端问题
 * @param  *send_cmd:   数据首地址
 * @param  cmd_num:     命令编号
 * @param  packet_len:  数据包长度(不包括数据头)
 * @retval None
 */
void  head_package(char *send_cmd,unsigned short cmd_num,unsigned int packet_len){
    //unsigned short cmd_num=0x0002;
	//unsigned int packet_len = sizeof(cmd_num)+sizeof(packet_len);
    //assert(sizeof(send_cmd)>=6);
    int i=0;
    //cmd_num: 0x00000006
	//cmd_num => send_cmd[0] send_cmd[1] 
    //              0x02       0x00
	//packet_len: 0x00000006
	//packet_len => send_cmd[2] send_cmd[3] send_cmd[4] send_cmd[5]
	//					0x06		0x00		0x00	 0x00
	//命令长度 ，小端模式存放
    send_cmd[i++] = cmd_num & 0xff;
    send_cmd[i++] = (cmd_num >>8) & 0xff;
	send_cmd[i++] = packet_len & 0xff;
	send_cmd[i++] = (packet_len >> 8 ) & 0xff;
	send_cmd[i++] = (packet_len >> 16) & 0xff;
	send_cmd[i++] = (packet_len >> 24) & 0xff;
}
/** 
 * @brief  解析包头
 * @note   暂时没有考虑大小端问题
 * @param  *p: 
 * @param  *cmd_num: 
 * @param  *packet_len: 
 * @retval None
 */
void head_analyze(char *p,unsigned short *cmd_num,unsigned int *packet_len)
{
    assert(p != NULL);

    *cmd_num=(p[0]&0xff) | ((p[1]<<8)&0xff);
    *packet_len= (p[2]&0xff)                 | 
                ( (p[3]<<8)&0xff00  )       |
                ((p[4]<<16)&0xff0000  )     |
                ((p[5]<<24)&0xff000000);
}
/** 
 * @brief  把一个 unsigned int 数转化为小端模式字符串存放
 * @note   
 * @param  *cmd: 数据首地址
 * @param  num:  要转化的数字
 * @example
 *      uintToString(str,0x12345678);
 *      则:str[0] = 0x78;str[1] = 0x56;str[2] = 0x34;str[3] = 0x12;
 * @retval 成功返回0,失败返回-1
 */
int uintToString(char *str,unsigned int num){
    if(str == NULL)
        return -1;
    signed int i = 0;
    //假设 num = 0x12345678
    /* 1.如果是小端模式,即num存储的方式是: 0x78 0x56 0x34 0x12 */
    if(IsBigEndian() == 0){
        for(i=0;i<4;i++){
            str[i] = num &0xff;
            num = num >> 8;
        }
    }
    /* 2.如果是大端模式,即num存储的方式是: 0x12 0x34 0x56 0x78 */
    else{
        for(i=3;i>=0;i--){
            str[i] = num &0xff;
            num = num >> 8;
        }
    }
    return 0;
}
int stringToUint(char *str,unsigned int *num){
    if(str == NULL)
        return -1;
    signed int i = 0;
    *num = 0;
    if(IsBigEndian() == 0){
        for(i=3;i>=0;i--){
            *num = *num << 8;
            *num |= (str[i] & 0xff);
        }
    }else{
        for(i=0;i<4;i++){
            *num = *num << 8;
            *num |= (str[i] & 0xff);
        }
    }
    return 0;
}

