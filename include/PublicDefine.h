/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    PublicDefine.h
 *@����    �������ݽṹ����
 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#pragma once
#include<stdio.h>
#include <iostream>
#include"winerror.h"
#define WIN32_LEAN_AND_MEAN
#include"Winsock2.h"



#define OutErr(a) std::cout << "error ��" << (a) << std::endl \
      << "������룺"<< WSAGetLastError() << std::endl \
      << "�����ļ���"<< __FILE__ << std::endl  \
      << "����������"<< __LINE__ << std::endl \

#define OutMsg(a) std::cout << (a) << std::endl;



#define PORT            5050            // �����˿�
#define LOCAL_HOST      "127.0.0.1"     // ���ػ�·��ַ
#define DATA_BUFSIZE    8192

#define MAX_LISTEN_QUEUE    200

#define MAX_CONNECT      3000

#define MAX_DATA_LEN    2048        // ���ݰ�����

#define SEND_DATA_LEN    4096        // �������ݰ�����


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

#define ON_LINE_TIME                 30000   // ÿ��30s����һ����������


/// �ṹ�嶨��
/*
 *@brief    ����IOCP���ض�����
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct {
    OVERLAPPED Overlapped;
    WSABUF DataBuf;
    CHAR Buffer[DATA_BUFSIZE];
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

/*
 *@brief    ����IOCP���ض��ṹ
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
    eSubmit = 0,     // 0 �ϴ�
    eTranspond,      // 1 ת��
    eAcquire,        // 2 ����
    eRegister,       // 3 ע��
    eVerify,         // 4 ��¼У��
    eBroadcast,      // 5 �㲥
    eExit,           // 6 �˳���¼
    eEnd = 0xFFFF,   // ��β���
} DataType;

#pragma pack(1)
/*
 *@brief    ���ݰ�ͷ
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _DataHead {

    unsigned short  _type;      // 0=�ϴ����� 1=ת������ 2=�������� 3=ע�� 4=��¼У�� 5=�㲥���� 6�˳�
    unsigned int    _node;      // �ͻ���ID
    unsigned long   _time;

    _DataHead()
    {
        memset(this, 0, sizeof(_DataHead));
    }

} TcpHead, Udp_Head;

/*
 *@brief    ���ݰ���
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
 *@brief    ��������
 *@author   GhY
 *@date     2024/07/24
 */
typedef struct _SendData {
    TcpHead _head;
    TcpBody _body;
} Tcp_SendData, Udp_SendData;

#pragma pack()


/*
 *@brief    socket���ӹ���
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
 *@brief    ͨ����Ϣ
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


