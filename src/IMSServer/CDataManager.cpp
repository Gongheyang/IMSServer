/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CDataManager.cpp
 *@描述    数据管理类
 *
 *@作者    GhY
 *@日期    2024年8月7日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "CDataManager.h"
#include "MySqlite.h"




CDataManager::CDataManager()
{
    Init();
}

CDataManager::~CDataManager()
{
}

void CDataManager::Init()
{
    m_vecUserInfos.clear();
}

CDataManager* CDataManager::Instance()
{
    static CDataManager instance;
    return &instance;
}

std::vector<UserInfo> CDataManager::GetUserInfos()
{
    return m_vecUserInfos;
}

void CDataManager::UpdateUserInfos(std::vector<UserInfo>& vec)
{
    m_vecUserInfos.clear();
    m_vecUserInfos = std::move(vec);
}

bool CDataManager::GetUserInfo(const int& id, UserInfo& info)
{
    bool bRet = false;
    auto tmp = m_vecUserInfos.begin();
    for (; tmp != m_vecUserInfos.end(); tmp++) {
        if (tmp->_id == id) {
            info = *tmp;
            bRet = true;
            break;
        }
    }
    return bRet;
}

void CDataManager::InsertUserInfo(const UserInfo& data)
{
    m_vecUserInfos.push_back(data);
}

void CDataManager::DeleteUserInfo(const int& id)
{
    auto tmp = m_vecUserInfos.begin();
    for (; tmp != m_vecUserInfos.end(); tmp++) {
        if (tmp->_id == id) {
            m_vecUserInfos.erase(tmp);
            break;
        }
    }
}

bool CDataManager::VerifyAccountWithPwd(const std::string& account, const std::string& pwd)
{
    bool bReturn = false;
    int id = atoi(account.c_str());
    auto tmp = m_vecUserInfos.begin();
    for (; tmp != m_vecUserInfos.end(); tmp++) {
        if (tmp->_id == id && (tmp->_pwd.compare(pwd) == 0)) {
            bReturn = true;
            break;
        }
    }
    return bReturn;
}

bool CDataManager::IsAccountIdExists(const int& accountId)
{
    bool bReturn = false;
    for (auto tmp = m_vecUserInfos.begin();
            tmp != m_vecUserInfos.end(); tmp++) {
        if (tmp->_id == accountId) {
            bReturn = true;
            break;
        }
    }
    return bReturn;
}

