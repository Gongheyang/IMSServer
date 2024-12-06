/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    WinPublicIO.cpp
 *@描述    完成服务器多线程 信息处理
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
#include "json/reader.h"



using namespace std;


DWORD WINAPI ReceiveProcessIO(LPVOID lpParam)
{
    CAppServer* app = (CAppServer*)lpParam;
    if (!app) {
        return 1;
    }
    HANDLE CompletionPort = app->m_completionPort;
    DWORD BytesTransferred = 0;
    LPPER_HANDLE_DATA PerHandleData;
    LPPER_IO_OPERATION_DATA PerIoData;

    while (true) {
        if (0 == GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&PerIoData, INFINITE)) {
            if ((GetLastError() == WAIT_TIMEOUT) || (GetLastError() == ERROR_NETNAME_DELETED)) {
                cout << "closingsocket" << PerHandleData->_socket << endl;
                int ccH = PerHandleData->_socket;
                closesocket(PerHandleData->_socket);
                delete PerIoData;
                delete PerHandleData;
                app->OnCloseSocketEvent.emit(ccH);
                continue;
            } else {
                OutErr("GetQueuedCompletionStatus failed!");
            }
            return 0;
        }

        // 说明客户端已经退出
        if (BytesTransferred == 0) {
            cout << "closing socket" << PerHandleData->_socket << endl;
            int ccH = PerHandleData->_socket;
            closesocket(PerHandleData->_socket);
            app->OnCloseSocketEvent.emit(ccH);
            delete PerIoData;
            delete PerHandleData;
            continue;
        }

        unsigned int currentId = 0;
        unsigned int iType = -1;
        // 取得数据并处理
        Tcp_SendData* pData = (Tcp_SendData*)PerIoData->Buffer;
        if (pData) {
            if (pData->_body._data != "" && pData->_body._length > 0) {
#ifdef _DEBUG
                cout << PerHandleData->_socket << "发送过来的消息：" << pData->_body._data
                     << " 信息长度：" << pData->_body._length << endl;
#endif
                MySRand rd;
                if (pData->_head._type == DataType::eRegister) {
                    currentId = app->CreateUserID();
                } else {
                    currentId = pData->_head._node;
                }

                iType = pData->_head._type;
                Message* msg = new Message();
                msg->_type = iType;
                msg->_sendId = currentId;
				memcpy(&msg->_send_name, &pData->_body._srcName, strlen(pData->_body._srcName));
				memcpy(&msg->_data, &pData->_body._data, pData->_body._length);
                if (iType == DataType::eTranspond) {
                    std::string receiverId = "";
                    std::string receiverName = "";
                    Json::Reader readJson;
                    Json::Value root_value;
                    std::string strData = msg->_data;
                    bool retJson = readJson.parse(strData, root_value);
                    if (!retJson) {
                        printf_s("[%s] [%d]: json parse receive data failure !!! \n", __FUNCTION__, __LINE__);
                    } else {
                        receiverId = root_value[SEND_ACCOUNT].asString();  // 接收者account
                        receiverName = root_value[SEND_ACCOUNT_NAME].asString();    // 接收者 name
                    }
                    if (!receiverId.empty()) {
                        msg->_receiverId = atoi(receiverId.c_str());
                    }
                    if (!receiverName.empty()) {
                        memcpy(&msg->_receiverName, receiverName.c_str(), receiverName.length());
                    }
                }
                if (iType > DataType::eAcquire) {
                    app->m_clientMessages.push_back(msg);
                } else {
                    app->m_tranMessages.push_back(msg);
                }
            }
        }

        ClientManager cManager;
        cManager._socket = PerHandleData->_socket;
        sprintf(cManager._addr, PerHandleData->_ip);
        cManager._port = PerHandleData->_port;
        cManager._id = currentId;
        if (app->m_connectSockets.find(cManager._socket) == app->m_connectSockets.end()) {
            app->m_connectSockets.insert(std::make_pair(cManager._socket, cManager));
        }
        //退出
        if (app->m_exitFlag) {
            break;
        }

        // 继续向 socket 投递WSARecv操作
        DWORD Flags = 0;
        DWORD dwRecv = 0;
        ZeroMemory(PerIoData, sizeof(PER_IO_OPERATION_DATA));
        PerIoData->DataBuf.buf = PerIoData->Buffer;
        PerIoData->DataBuf.len = DATA_BUFSIZE;
        WSARecv(PerHandleData->_socket, &PerIoData->DataBuf, 1, &dwRecv, &Flags, &PerIoData->Overlapped, NULL);

    }

    return 0;
}

DWORD WINAPI ClientMessageProcess(LPVOID lpParam)
{
    CAppServer* appServer = (CAppServer*)lpParam;
    if (!appServer) {
        return 0;
    }
    while (1) {
        if (appServer->m_clientMessages.empty()) {
            if (appServer->m_exitFlag) {
                break;
            }
            Sleep(300);
            continue;
        }
        Message* msg = appServer->m_clientMessages.front();
        for (auto con = appServer->m_connectSockets.begin(); con != appServer->m_connectSockets.end(); con++) {
            if (msg->_type == DataType::eRegister) {
                if (appServer->DealWithRegisterAccount(msg, &con->second)) {
                    break;
                }
            } else if (msg->_type == DataType::eVerify) {
                if (appServer->DealWithAccountVerify(msg, &con->second)) {
                    break;
                }
            } else if (msg->_type == DataType::eBroadcast) {
                appServer->DealWithBroadcastMsg(DataType::eBroadcast, msg, &con->second);
                continue;
            } else if (msg->_type == DataType::eExit) {
                appServer->DealWithBroadcastMsg(DataType::eExit, msg, &con->second);
                continue;
            } else {
                break;
            }
        }
        delete msg;
        msg = nullptr;
        appServer->m_clientMessages.pop_front();
        if (appServer->m_exitFlag) {
            break;
        }
    }
    return 0;
}

DWORD WINAPI TranspondMessageProcess(LPVOID lpParam)
{
    CAppServer* appServer = (CAppServer*)lpParam;
    if (!appServer) {
        return 0;
    }
    while (1) {
        if (appServer->m_tranMessages.empty()) {
            if (appServer->m_exitFlag) {
                break;
            }
            Sleep(300);
            continue;
        }
        Message* msg = appServer->m_tranMessages.front();
        for (auto con = appServer->m_connectSockets.begin(); con != appServer->m_connectSockets.end(); con++) {
            if (msg->_type == DataType::eSubmit) {
                break;
            } else if (msg->_type == DataType::eTranspond) {
                if (appServer->DealWithTranspondMsg(msg, &con->second)) {
                    break;
                }
            } else if (msg->_type == DataType::eAcquire) {
                continue;
            } else {
                break;
            }
            //if (msg->_sendId != con->second._id) {
            //    char sendBuf[SEND_DATA_LEN];
            //    memset(sendBuf, 0, sizeof(sendBuf));
            //    int ihead = sizeof(TcpHead);
            //    int ibody = sizeof(TcpBody);
            //    Tcp_SendData* pData = (Tcp_SendData*)sendBuf;
            //    pData->_head._node = 0;
            //    pData->_head._type = 1;
            //    pData->_head._time = time(NULL);
            //    TcpBody tBody;
            //    std::string tmp = msg->_data;
            //    std::string tmpName = msg->_send_name;
            //    tBody._length = tmp.length();
            //    memcpy(tBody._srcName, tmpName.c_str(), tmpName.length());
            //    memcpy(tBody._data, tmp.c_str(), tmp.length());
            //    memcpy(&sendBuf[ihead], &tBody, ibody);
            //    int sendLen = ihead + ibody;
            //    int ret = send(con->second._socket, sendBuf, sendLen, 0);
            //} else {
            //    continue;
            //}
        }
        delete msg;
        msg = nullptr;
        appServer->m_tranMessages.pop_front();
        if (appServer->m_exitFlag) {
            break;
        }

    }
    return 0;
}


