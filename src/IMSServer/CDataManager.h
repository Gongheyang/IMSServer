/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CDataManager.h
 *@描述    数据管理类
 *
 *@作者    GhY
 *@日期    2024年8月7日
 *@版本    v1.0.0
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
    *@brief    单例接口
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


