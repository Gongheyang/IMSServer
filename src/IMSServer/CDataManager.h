/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    CDataManager.h
 *@����    ���ݹ�����
 *
 *@����    GhY
 *@����    2024��8��7��
 *@�汾    v1.0.0
 *
 ****************************************************/
#ifndef __CDATAMANAGER_H__
#define __CDATAMANAGER_H__
#include <string>
#include <vector>
#include "PublicDefine.h"




class CDataManager
{
public:

    /*
    *@brief    �����ӿ�
    *@author   GhY
    *@date     2024/07/24
    */
    static CDataManager* Instance();

    std::vector<UserInfo> GetUserInfos();

    void  UpdateUserInfos(std::vector<UserInfo>& vec);

    bool GetUserInfo(const int& id, UserInfo& info);

    void InsertUserInfo(const UserInfo& data);

    void DeleteUserInfo(const int& id);

    bool VerifyAccountWithPwd(const std::string& account, const std::string& pwd);

    bool IsAccountIdExists(const int& accountId);
protected:

    CDataManager();
    ~CDataManager();

    void Init();


private:

    std::vector<UserInfo> m_vecUserInfos;



};

#define g_DataMangerPtr (*(CDataManager::Instance()))


#endif //!__CDATAMANAGER_H__


