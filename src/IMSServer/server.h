/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    server.h
 *@����    ��ɶ˿ڷ����� ���յ��ͻ��˵���Ϣ
 *
 *@����    GhY
 *@����    2024��7��17��
 *@�汾    v1.0.0
 *
 ****************************************************/
#include "PublicDefine.h"
#include"windows.h"
#include "sigslot.h"
#include <map>
#include "fylock.h"


/*
 *@����:    �������
 *@����:    GhY
 *@����:    2024/07/24
 *@��ʷ:
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
     *@desc       ��ʼ��WinSocket
     *@param:
     *@return
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    void InitWinsock();

    /*
     *@desc       ����һ���Ѵ򿪵��ļ�ʵ�����½��Ļ��Ѵ��ڵ�I/0��ɶ˿�
                  ���ߴ���һ��δ�����κ��ļ���I/O��ɶ˿�
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    void InitCompletionPort();

    /*
     *@desc       �󶨶˿ڣ�������һ�� Overlapped ��ListenSocket
     *@param:     int nPort
     *@return     socket
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    SOCKET BindServerOverlapped(int nPort);

    /*
     *@desc       ����ϵͳ��CPU�������������߳�
     *@author     GhY
     *@date       2024/07/17
     *@version    v1.0.0
     *@history:
     */
    void CreateMultiThread();

    /*
     *@brief    ������ user ID
     *@author   GhY
     *@date     2024/08/08
     */
    int CreateUserID();

    /*
     *@desc       ����ע���˺�ҵ��
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithRegisterAccount(const Message* msg, const ClientManager* cmanager);
    /*
     *@desc       �����˺�У��ҵ��
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithAccountVerify(const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       ������Ϣת��ҵ��
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool DealWithTranspondMsg(const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       ����㲥��Ϣҵ��
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    void DealWithBroadcastMsg(const DataType& type, const Message* msg, const ClientManager* cmanager);

    /*
     *@desc       �����˻�����У��
     *@return     ture У��ͨ����false У��ʧ��
     *@author     GhY
     *@date       2024/08/08
     *@version    v1.0.0
     *@history:
     */
    bool VerifyAccountWithPwd(const std::string& account, const std::string& pwd);

    /*
     *@brief    ׼������
     *@author   GhY
     *@date     2024/07/24
     */
    void Prepare();

    void Run();

    void Close();

    /*
     *@desc       �ر�socket����
     *@param:     socket ���
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

    bool m_exitFlag;      // �˳���־

protected:
private:
    SOCKET m_serverListen;  // ����socket

    CCrtSection critical_section1_;
    CCrtSection critical_section2_;
    CCrtSection critical_section3_;
    CCrtSection critical_section4_;
};


