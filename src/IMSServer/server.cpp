/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    server.cpp
 *@描述    完成端口服务器 接收到客户端的信息
 *
 *@作者    GhY
 *@日期    2024年7月17日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "server.h"
#ifdef WIN32
#pragma comment(lib, "ws2_32")
#include"windows.h"
#include <Mstcpip.h>
#else
#include <netinet/tcp.h>
#endif

#include<iostream>
#include "MySRand.h"
#include <stdlib.h>
#include <time.h>
#include "CIniConfig.h"
#include "json/reader.h"
#include "CDataManager.h"
#include "MySqlite.h"
#include "WinPublicIO.h"



using namespace std;



CAppServer::CAppServer()
    : m_serverListen(NULL)
    , m_exitFlag(false)
{
    m_tranMessages.clear();
    OnCloseSocketEvent.connect(this, &CAppServer::OnCloseConnect);
}

CAppServer::~CAppServer()
{
    Close();
}

void CAppServer::InitWinsock()
{
    // 初始化WINSOCK
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0) {
        OutErr("WSAStartup()");
    }
}

void CAppServer::InitCompletionPort()
{
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_completionPort == NULL) {
        // 创建失败，处理错误
        OutErr(" CreateIoCompletionPort ");
        return ;
    }
}

SOCKET CAppServer::BindServerOverlapped(int nPort)
{
    // 创建socket
    m_serverListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    // 绑定端口
    char cz[8] = { 0 };
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(nPort);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_serverListen, (struct sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        OutErr("bind Failed!");
        return NULL;
    }



    int iOptVal = 0;
    int iOptLen = sizeof(int);
    BOOL bOptVal = true;
    int bOptLen = sizeof(BOOL);
    /*
    * 初始化变量并调用getsockopt。
    * SO_KEEPALIVE参数是一个套接字选项，它使套接字在会话中发送keepalive消息。
    * SO_KEEPALIVE套接字选项需要将一个布尔值传递给setsockopt函数。
    * 如果为TRUE, socket被配置为发送keepalive消息;如果为FALSE, socket被配置为不发送keepalive消息。
    * 这段代码通过使用getsockopt函数检查套接字上SO_KEEPALIVE的状态来测试setsockopt函数。
    */
    int iResult = getsockopt(m_serverListen, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    } else {
        wprintf(L"SO_KEEPALIVE Value: %ld\n", iOptVal);
    }

    iResult = setsockopt(m_serverListen, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    } else {
        wprintf(L"Set SO_KEEPALIVE: ON\n");
    }

    iResult = getsockopt(m_serverListen, SOL_SOCKET, SO_KEEPALIVE, (char*)&iOptVal, &iOptLen);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"getsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    } else {
        wprintf(L"SO_KEEPALIVE Value: %ld\n", iOptVal);
    }

    tcp_keepalive alive_in = { 0 };
    tcp_keepalive alive_out = { 0 };
    // keepalivetime     成员指定超时（以毫秒为单位）在发送第一个 keep-alive 数据包之前没有活动。
    // keepaliveinterval 成员指定在未收到确认的情况下发送连续保持连接数据包之间的间隔（以毫秒为单位）。
    alive_in.keepalivetime = 2 * 60 * 1000;          // 开始首次KeepAlive探测前的TCP空闭时间 2分钟
    alive_in.keepaliveinterval = 3 * 60 * 1000;
    alive_in.onoff = TRUE;
    unsigned long ulBytesReturn = 0;
    int nRet = WSAIoctl(m_serverListen, SIO_KEEPALIVE_VALS, &alive_in,
                        sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);

    if (nRet == SOCKET_ERROR) {
        wprintf(L"WSAIoctl SIO_KEEPALIVE_VALS failed with error:: %u\n", WSAGetLastError());
    }


    // 设置监听队列为 MAX_LISTEN_QUEUE
    if (listen(m_serverListen, MAX_LISTEN_QUEUE) != 0) {
        OutErr("listen Failed!");
        return NULL;
    }
    return m_serverListen;

}

void CAppServer::CreateMultiThread()
{
    //根据系统的CPU来创建工作者线程
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    //线程数目=系统进程数目的两倍.
    for (int i = 0; i < SystemInfo.dwNumberOfProcessors /** 2*/; i++) {
        HANDLE hProcessIO = CreateThread(NULL, 0, ReceiveProcessIO, this, 0, NULL);
        if (hProcessIO) {
            CloseHandle(hProcessIO);
        }
    }

    HANDLE hTranProcess = CreateThread(NULL, 0, TranspondMessageProcess, this, 0, NULL);
    if (hTranProcess) {
        CloseHandle(hTranProcess);
    }

    HANDLE hClientProcess = CreateThread(NULL, 0, ClientMessageProcess, this, 0, NULL);
    if (hClientProcess) {
        CloseHandle(hClientProcess);
    }

}

int CAppServer::CreateUserID()
{
    MySRand rd;
    int numId = atoi(rd.getNumber().c_str());
    if (g_DataMangerPtr.IsAccountIdExists(numId)) {
        numId = CreateUserID();
    }
    return numId;
}

void CAppServer::Prepare()
{
    InitWinsock();

    InitCompletionPort();

    //读取数据库
    g_MySqlitePtr.SelectAllUserinfo();

    CreateMultiThread();

    std::string sPort = g_ConfigPtr.getConfigValueWithKey(CONFIG_NET, CONFIG_PORT);
    int iPort = sPort.empty() ? PORT : atoi(sPort.c_str());
    printf("listen port: %d \n", iPort);
    BindServerOverlapped(iPort);    
}

void CAppServer::Run()
{
    SOCKET sClient;
    LPPER_HANDLE_DATA PerHandleData;
    LPPER_IO_OPERATION_DATA PerIoData;

    while (true) {
        if (!m_serverListen) {
            break;
        }
        if (m_exitFlag) {
            break;
        }
        // 等待客户端接入
        //sClient = WSAAccept(sListen, NULL, NULL, NULL, 0);
        sockaddr_in remoteAddr;
        int nAddrlen = sizeof(remoteAddr);

        sClient = accept(m_serverListen, (SOCKADDR*)&remoteAddr, &nAddrlen);
        if (sClient == INVALID_SOCKET) {
            continue;
        }

        cout << endl;
        cout << "Socket " << sClient << "连接进来" << endl;

        PerHandleData = new PER_HANDLE_DATA();
        PerHandleData->_socket = sClient;
        sprintf(PerHandleData->_ip, inet_ntoa(remoteAddr.sin_addr));
        PerHandleData->_port = remoteAddr.sin_port;

        // 将接入的客户端和完成端口联系起来
        CreateIoCompletionPort((HANDLE)sClient, m_completionPort, (DWORD)PerHandleData, 0);

        // 建立一个Overlapped，并使用这个Overlapped结构对socket投递操作
        PerIoData = new PER_IO_OPERATION_DATA();

        ZeroMemory(PerIoData, sizeof(PER_IO_OPERATION_DATA));
        PerIoData->DataBuf.buf = PerIoData->Buffer;
        PerIoData->DataBuf.len = DATA_BUFSIZE;

        // 投递一个WSARecv操作
        DWORD Flags = 0;
        DWORD dwRecv = 0;
        WSARecv(sClient, &PerIoData->DataBuf, 1, &dwRecv, &Flags, &PerIoData->Overlapped, NULL);
    }

    DWORD dwByteTrans = 0;
    //将一个已经完成的IO通知添加到IO完成端口的队列中.
    //提供了与线程池中的所有线程通信的方式.
    PostQueuedCompletionStatus(m_completionPort, dwByteTrans, 0, 0); //IO操作完成时接收的字节数.
}

void CAppServer::Close()
{
    m_exitFlag = true;
    if (m_serverListen) {
        closesocket(m_serverListen);
        m_serverListen = NULL;
    }
#ifdef _MSC_VER
    if (m_completionPort) {
        PostQueuedCompletionStatus(m_completionPort, 0, 0, 0);
        m_completionPort = NULL;
    }
#endif
    OnCloseSocketEvent.disconnect_all();

}

void CAppServer::OnCloseConnect(int handle)
{
    auto tmp = m_connectSockets.find(handle);
    if (tmp != m_connectSockets.end()) {
        m_connectSockets.erase(tmp);
    }
}


bool CAppServer::DealWithRegisterAccount(const Message* msg, const ClientManager* cmanager)
{
    //给客户端分配唯一id
    if (msg->_sendId == cmanager->_id) {
        CAutoLockEx<CCrtSection> lock(critical_section1_);
        char sendBuf2[SEND_DATA_LEN];
        memset(sendBuf2, 0, sizeof(sendBuf2));
        int ihead = sizeof(TcpHead);
        int ibody = sizeof(TcpBody);
        Tcp_SendData* pData2 = (Tcp_SendData*)sendBuf2;
        pData2->_head._node = cmanager->_id;
        pData2->_head._type = DataType::eRegister;
        pData2->_head._time = time(NULL);
        TcpBody tBody;
        Json::Value root;
        root[SEND_RESULT] = true;
        root[SEND_ACCOUNT] = std::to_string(cmanager->_id).c_str();
        std::string strSendData = root.toStyledString();
        tBody._length = strSendData.length();
        memcpy(tBody._data, strSendData.c_str(), strSendData.length());
        memcpy(&sendBuf2[ihead], &tBody, ibody);
        int sendLen = ihead + ibody;
        int ret = send(cmanager->_socket, sendBuf2, sendLen, 0);

        std::string strPwd = "";
        std::string strMac = "";
        Json::Reader readJson;
        Json::Value root_value;
        std::string strData = msg->_data;
        bool retJson = readJson.parse(strData, root_value);
        if (!retJson) {
            printf_s("json parse receive data failure !!! \n");
        } else {
            strPwd = root_value[SEND_PASSWORD].asString();
            strMac = root_value[SEND_MAC].asString();
        }

        UserInfo info;
        info._id = msg->_sendId;
        info._name = msg->_send_name;
        info._pwd = strPwd;
        info._mac = strMac;
        g_DataMangerPtr.InsertUserInfo(info);
        g_MySqlitePtr.InsertUserInfo(info);
        return true;
    }
    return false;
}

bool CAppServer::DealWithAccountVerify(const Message* msg, const ClientManager* cmanager)
{
    if (msg == nullptr) {
        return false;
    }
    if (msg->_sendId == cmanager->_id) {
        CAutoLockEx<CCrtSection> lock(critical_section2_);
        //进行账号校验
        std::string strAccount = "";
        std::string strPwd = "";
        Json::Reader readJson;
        Json::Value root_value;
        std::string strData = msg->_data;
        bool retJson = readJson.parse(strData, root_value);
        if (!retJson) {
            printf_s("json parse receive data failure !!! \n");
        } else {
            strAccount = root_value[SEND_ACCOUNT].asString();
            strPwd = root_value[SEND_PASSWORD].asString();
        }
        bool bRet = VerifyAccountWithPwd(strAccount, strPwd);

        char sendBuf[SEND_DATA_LEN];
        memset(sendBuf, 0, sizeof(sendBuf));
        int ihead = sizeof(TcpHead);
        int ibody = sizeof(TcpBody);
        Tcp_SendData* pData = (Tcp_SendData*)sendBuf;
        pData->_head._node = msg->_sendId;
        pData->_head._type = DataType::eVerify;
        pData->_head._time = time(NULL);
        TcpBody tBody;
        std::string strVerifyResult = bRet ? "verify success" : "verify failed";
        Json::Value root;
        root[SEND_VERIFY] = bRet;
        root[SEND_MSG] = strVerifyResult.c_str();
        std::string strSendData = root.toStyledString();

        std::string tmpName = msg->_send_name;
        tBody._length = strSendData.length();
        memcpy(tBody._srcName, tmpName.c_str(), tmpName.length());
        memcpy(tBody._data, strSendData.c_str(), strSendData.length());
        memcpy(&sendBuf[ihead], &tBody, ibody);
        int sendLen = ihead + ibody;

        int ret = send(cmanager->_socket, sendBuf, sendLen, 0);
        printf("send buf len : %d\n ", ret);
        return true;
    }
    return false;
}

bool CAppServer::DealWithTranspondMsg(const Message* msg, const ClientManager* cmanager)
{
    if (msg->_receiverId == cmanager->_id) {
        CAutoLockEx<CCrtSection> lock(critical_section3_);
        char sendBuf[SEND_DATA_LEN];
        memset(sendBuf, 0, sizeof(sendBuf));
        int ihead = sizeof(TcpHead);
        int ibody = sizeof(TcpBody);
        Tcp_SendData* pData = (Tcp_SendData*)sendBuf;
        pData->_head._node = msg->_sendId;
        pData->_head._type = DataType::eTranspond;
        pData->_head._time = time(NULL);
        TcpBody tBody;
        std::string tmp = msg->_data;
        std::string tmpName = msg->_send_name;
        tBody._length = tmp.length();
        memcpy(tBody._srcName, tmpName.c_str(), tmpName.length());
        memcpy(tBody._data, tmp.c_str(), tmp.length());
        memcpy(&sendBuf[ihead], &tBody, ibody);
        int sendLen = ihead + ibody;
        int ret = send(cmanager->_socket, sendBuf, sendLen, 0);
        return true;
    }
    return false;
}

void CAppServer::DealWithBroadcastMsg(const DataType& type, const Message* msg, const ClientManager* cmanager)
{
    if (msg->_sendId != cmanager->_id) {
        CAutoLockEx<CCrtSection> lock(critical_section4_);
        char sendBuf[SEND_DATA_LEN];
        memset(sendBuf, 0, sizeof(sendBuf));
        int ihead = sizeof(TcpHead);
        int ibody = sizeof(TcpBody);
        Tcp_SendData* pData = (Tcp_SendData*)sendBuf;
        pData->_head._node = msg->_sendId;
        pData->_head._type = type;
        pData->_head._time = time(NULL);
        TcpBody tBody;
        std::string tmp = msg->_data;
        std::string tmpName = msg->_send_name;
        tBody._length = tmp.length();
        memcpy(tBody._srcName, tmpName.c_str(), tmpName.length());
        memcpy(tBody._data, tmp.c_str(), tmp.length());
        memcpy(&sendBuf[ihead], &tBody, ibody);
        int sendLen = ihead + ibody;
        int ret = send(cmanager->_socket, sendBuf, sendLen, 0);
    }

}

bool CAppServer::VerifyAccountWithPwd(const std::string& account, const std::string& pwd)
{
    if (account.empty() || pwd.empty()) {
        return false;
    }
    return g_DataMangerPtr.VerifyAccountWithPwd(account, pwd);
}

