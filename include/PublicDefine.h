/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    PublicDefine.h
 *@描述    公共数据结构定义
 *
 *@作者    GhY
 *@日期    2024年7月24日
 *@版本    v1.0.0
 *
 ****************************************************/
#pragma once
#include<stdio.h>
#include <iostream>
#include"winerror.h"
#define WIN32_LEAN_AND_MEAN
#include"Winsock2.h"



#define OutErr(a) std::cout << "error ：" << (a) << std::endl \
      << "出错代码："<< WSAGetLastError() << std::endl \
      << "出错文件："<< __FILE__ << std::endl  \
      << "出错行数："<< __LINE__ << std::endl \

#define OutMsg(a) std::cout << (a) << std::endl;



#define PORT            5050            // 监听端口
#define LOCAL_HOST      "127.0.0.1"     // 本地回路地址
#define DATA_BUFSIZE    8192

#define MAX_LISTEN_QUEUE    200

#define MAX_CONNECT      3000

#define MAX_DATA_LEN    2048        // 数据包长度

#define SEND_DATA_LEN    4096        // 发送数据包长度


#define CONFIG_NET                  "net"
#define CONFIG_IP                   "ip"
#define CONFIG_PORT                 "port"

#define CONFIG_BASE                 "base"
#define CONFIG_ACCOUNT              "account"
#define CONFIG_NAME                 "name"
#define CONFIG_PWD                  "password"


#define SEND_TYPE                   "type"
#define SEND_ACCOUNT                "account"
#define SEND_ACCOUNT_NAME           "name"
#define SEND_PASSWORD               "pwd"
#define SEND_VERIFY                 "verify"
#define SEND_MSG                    "msg"
#define SEND_RESULT                 "result"
#define SEND_MAC                    "mac"
#define SEND_HEARTBEAT              "heartbeat"

#define ON_LINE_TIME                 30000   // 每隔30s发送一次在线心跳


/// 结构体定义
/*
 *@brief    用于IOCP的特定函数
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct {
    OVERLAPPED Overlapped;
    WSABUF DataBuf;
    CHAR Buffer[DATA_BUFSIZE];
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

/*
 *@brief    用于IOCP的特定结构
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _PER_HANDLE_DATA {
    SOCKET _socket;
    CHAR _ip[32];
    int _port;

    _PER_HANDLE_DATA()
    {
        _socket = NULL;
        memset(_ip, 0, 32);
        _port = -1;
    }
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


typedef enum _DataType {
    eSubmit = 0,     // 0 上传
    eTranspond,      // 1 转发
    eAcquire,        // 2 请求
    eRegister,       // 3 注册
    eVerify,         // 4 登录校验
    eBroadcast,      // 5 广播
    eExit,           // 6 退出登录
    eEnd = 0xFFFF,   // 结尾标记
} DataType;

#pragma pack(1)
/*
 *@brief    数据包头
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _DataHead {

    unsigned short  _type;      // 0=上传数据 1=转发数据 2=请求数据 3=注册 4=登录校验 5=广播数据 6退出
    unsigned int    _node;      // 客户端ID
    unsigned long   _time;

    _DataHead()
    {
        memset(this, 0, sizeof(_DataHead));
    }

} TcpHead, Udp_Head;

/*
 *@brief    数据包体
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _DataBody {
    char        _srcName[32];
    int         _length;
    char        _data[MAX_DATA_LEN];
    _DataBody()
    {
        memset(this, 0, sizeof(_DataBody));
    }

} TcpBody, UdpBody;

/*
 *@brief    发送数据
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _SendData {
    TcpHead _head;
    TcpBody _body;
} Tcp_SendData, Udp_SendData;

#pragma pack()


/*
 *@brief    socket连接管理
 *@author   GhY
 *@date     2024/07/24
 */
struct ClientManager {
    unsigned int _id;
    char _name[32];
    SOCKET _socket;
    char _addr[16];
    int _port;

    ClientManager()
    {
        memset(this, 0, sizeof(ClientManager));
    }

};

/*
 *@brief    通信消息
 *@author   GhY
 *@date     2024/07/24
 */
struct Message {
    unsigned int _type;
    unsigned int _sendId;
    char _send_name[32];
    unsigned int _receiverId;
    char _receiverName[32];
    char _data[MAX_DATA_LEN];

    Message()
    {
        memset(this, 0, sizeof(Message));
    }
};


class UserInfo
{
public:
    int _id;
    std::string _name;
    std::string _pwd;
    std::string _mac;

    UserInfo()
    {
        _id = 0;
        _name = "";
        _pwd = "";
        _mac = "";
    }
    UserInfo(int id, std::string name, std::string pwd, std::string mac)
        : _id(id)
        , _name(name)
        , _pwd(pwd)
        , _mac(mac) {}

    UserInfo(const UserInfo& o)
        : _id(o._id)
        , _name(o._name)
        , _pwd(o._pwd)
        , _mac(o._mac) {}

    UserInfo& operator= (const UserInfo& o)
    {
        _id = o._id;
        _name = o._name;
        _pwd = o._pwd;
        _mac = o._mac;
        return *this;
    }
};

class ClientInfo
{
public:
    std::string _account;
    std::string _name;
    bool _online;

    ClientInfo()
    {
        _account = "";
        _name = "";
        _online = false;
    }
    ClientInfo(std::string account, std::string name, bool online)
        : _account(account)
        , _name(name)
        , _online(online) {}

    ClientInfo(const ClientInfo& o)
        : _account(o._account)
        , _name(o._name)
        , _online(o._online) {}

    ClientInfo& operator= (const ClientInfo& o)
    {
        _account = o._account;
        _name = o._name;
        _online = o._online;
        return *this;
    }
};


