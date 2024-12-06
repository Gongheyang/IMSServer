/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    MySqlite.cpp
 *@����    MySqlite��ʵ��

 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#include <stdio.h>
#include "MySqlite.h"
#include "CIniConfig.h"
#include "CDataManager.h"



MySqlite::MySqlite()
    : m_db(NULL)
{
    InitDB();

}

MySqlite::~MySqlite()
{
    if (m_db) {
        sqlite3_close(m_db);
    }
}

MySqlite* MySqlite::Instance()
{
    static MySqlite instance;
    return &instance;
}

void MySqlite::InitDB()
{
    int rc;
    std::string currentPath = g_ConfigPtr.GetRootPath();
    currentPath.append("/../data/sqlite3.db");
    //���ʹ��sqlite3_open��sqlite3_open_v2�Ļ�,���ݿ⽫����UTF-8�ı��뷽ʽ,sqlite3_open16����UTF-16�ı��뷽ʽ
    rc = sqlite3_open(currentPath.c_str(), &m_db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s \t %s \n", currentPath.c_str(), sqlite3_errmsg(m_db));
        exit(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
}

bool MySqlite::SelectAllUserinfo()
{
    std::string strSql = "SELECT * from userinfos";
    return ExecuteSql(strSql.c_str(), SelectAllCallback);
}


bool MySqlite::InsertUserInfo(const UserInfo& info)
{
    std::string strSql = "INSERT INTO userinfos(id, name, pwd, mac) VALUES('";
    strSql.append(std::to_string(info._id) + std::string("', '"));
    strSql.append(info._name + std::string("', '"));
    strSql.append(info._pwd + std::string("', '"));
    strSql.append(info._mac + std::string("'); "));

    return ExecuteSql(strSql.c_str(), NonQueryCallBack);

}

bool MySqlite::UpdateUserInfo(const UserInfo& info)
{
    std::string strSql = "UPDATE userinfos set name = '"; ///(id, name, pwd, mac) VALUES('";

    strSql.append(info._name + std::string("',  pwd = '"));
    strSql.append(info._pwd + std::string("', mac = '"));
    strSql.append(info._mac + std::string("' where id = '"));
    strSql.append(std::to_string(info._id) + std::string("'; "));

    return ExecuteSql(strSql.c_str(), NonQueryCallBack);

}

bool MySqlite::DeleteUserInfo(const int& id)
{
    std::string strSql = "DELETE from userinfos where id= " + std::to_string(id);

    return ExecuteSql(strSql.c_str(), NonQueryCallBack);

}

int MySqlite::NonQueryCallBack(void* pv, int argc, char** argv, char** col)
{
    return 0;
}

int MySqlite::SelectAllCallback(void* NotUsed, int argc, char** argv, char** azColName)
{
    int i = 0;
    UserInfo info;
    for (i = 0; i < argc; i++) {
        std::string str = azColName[i];
        if (str == "id") {
            info._id = argv[i] ? atoi(argv[i]) : 0;
        } else if (str == "name") {
            info._name = argv[i] ? argv[i] : "";
        } else if (str == "pwd") {
            info._pwd = argv[i] ? argv[i] : "";
        } else if (str == "mac") {
            info._mac = argv[i] ? argv[i] : "";
        } else {
            printf("not find %s = %s \n", str.c_str(), argv[i] ? argv[i] : "NULL");
        }
    }
    g_DataMangerPtr.InsertUserInfo(info);

    return 0;
}

bool MySqlite::ExecuteSql(const char* sql, sqlite3_callback func)
{
    bool bRet = false;
    char* zErrMsg = 0;

    /* Execute SQL statement */
    int rc = sqlite3_exec(m_db, sql, func, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Execute Sql successfully\n");
        bRet = true;
    }
    return bRet;
}

