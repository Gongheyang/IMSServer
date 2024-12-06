/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    server.h
 *@描述    完成端口服务器 接收到客户端的信息
 *
 *@作者    GhY
 *@日期    2024年7月17日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "PublicDefine.h"
#include"windows.h"
#include "sigslot.h"
#include <map>
#include "fylock.h"


/*
 *@描述:    服务端类
 *@作者:    GhY
 *@日期:    2024/07/24
 *@历史:
 */
class  CAppServer : public sigslot::has_slots<>
{
public:
    typedef sigslot::signal1<int> HandleEvent;

    HandleEvent OnCloseSocketEvent;

public:

    CAppServer();

    ~CAppServer();

    /*
     *@desc       初始化WinSocket
     *@param:
     *@return
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    void InitWinsock();

    /*
     *@desc       关联一个已打开的文件实例和新建的或已存在的I/0完成端口
                  或者创建一个未关联任何文件的I/O完成端口
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    void InitCompletionPort();

    /*
     *@desc       绑定端口，并返回一个 Overlapped 的ListenSocket
     *@param:     int nPort
     *@return     socket
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    SOCKET BindServerOverlapped(int nPort);

    /*
     *@desc       根据系统的CPU来创建工作者线程
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    void CreateMultiThread();

    /*
     *@brief    生成新 user ID
     *@author   GhY
     *@date     2024/08/08
     */
    int CreateUserID();

    /*
     *@desc       处理注册账号业务
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithRegisterAccount(const Message* msg, const ClientManager* cmanager);
    /*
     *@desc       处理账号校验业务
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithAccountVerify(const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       处理消息转发业务
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithTranspondMsg(const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       处理广播消息业务
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    void DealWithBroadcastMsg(const DataType& type, const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       处理账户密码校验
     *@return     ture 校验通过；false 校验失败
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool VerifyAccountWithPwd(const std::string& account, const std::string& pwd);

    /*
     *@brief    准备函数
     *@author   GhY
     *@date     2024/07/24
     */
    void Prepare();

    void Run();

    void Close();

    /*
     *@desc       关闭socket连接
     *@param:     socket 句柄
     *@return
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    void OnCloseConnect(int);

public:
    HANDLE m_completionPort;
    std::map<unsigned int, ClientManager>  m_connectSockets;
    std::list<Message*> m_tranMessages;
    std::list<Message*> m_clientMessages;

    bool m_exitFlag;      // 退出标志

protected:
private:
    SOCKET m_serverListen;  // 监听socket

    CCrtSection critical_section1_;
    CCrtSection critical_section2_;
    CCrtSection critical_section3_;
    CCrtSection critical_section4_;
};


