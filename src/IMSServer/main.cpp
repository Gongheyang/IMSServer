#include "server.h"


#include "MySqlite.h"


void testsql()
{
    //MySqlite sqlite;

    ////char* sql = "INSERT INTO userinfos (id,name,pwd,mac) VALUES('100002','小明','123123','1C:7E:51:A1:21:65'); ";

    //char* sql = "SELECT * from userinfos";
    //g_MySqlitePtr.SelectAllUserinfo();

    //UserInfo info(456789, "明明", "123123", "1C:7E:51:A1:21:65");
    //g_MySqlitePtr.InsertUserInfo(info);
    //UserInfo info2(456555, "帆帆", "123123", "1C:7E:51:A1:21:65");
    //g_MySqlitePtr.InsertUserInfo(info2);

    //UserInfo info3(456789, "大明", "123456", "1C:7E:51:A1:21:65");
    //g_MySqlitePtr.UpdateUserInfo(info3);

    //g_MySqlitePtr.DeleteUserInfo(456789);
    //g_MySqlitePtr.DeleteUserInfo(456555);


}

int main()
{
    CAppServer* appS = new CAppServer();

    appS->Prepare();

    appS->Run();

    return 0;
}

