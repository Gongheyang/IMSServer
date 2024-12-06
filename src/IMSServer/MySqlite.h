/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    MySqlite.h
 *@����    MySqlite������
 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#ifndef __MYSQLITE_H__
#define __MYSQLITE_H__
#include <string>
#include "sqlite3/sqlite3.h"
#include "PublicDefine.h"


class MySqlite
{
public:

    /*
    *@brief    �����ӿ�
    *@author   GhY
    *@date     2024/07/24
    */
    static MySqlite* Instance();

    bool SelectAllUserinfo();

    /*
     *@brief    ����һ���û���Ϣ
     *@author   GhY
     *@date     2024/09/02
     */
    bool InsertUserInfo(const UserInfo& info);

    /*
     *@brief    ����һ���û���Ϣ
     *@author   GhY
     *@date     2024/09/02
     */
    bool UpdateUserInfo(const UserInfo& info);

    /*
     *@brief   ɾ��һ���û���Ϣ
     *@author   GhY
     *@date     2024/09/02
     */
    bool DeleteUserInfo(const int& id);

protected:

    MySqlite();
    ~MySqlite();

    void InitDB();

    bool ExecuteSql(const char* sql, sqlite3_callback func);

    static int NonQueryCallBack(void* pv, int argc, char** argv, char** col);

    static int SelectAllCallback(void* NotUsed, int argc, char** argv, char** azColName);

private:

    sqlite3* m_db;
};


#define g_MySqlitePtr (*(MySqlite::Instance()))

#endif // !__MYSQLITE_H__

