/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    WinPublicIO.h
 *@描述    完成服务器 多线程信息处理
 *
 *@作者    GhY
 *@日期    2024年7月17日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "PublicDefine.h"
#include"windows.h"
#include <map>


/*
 *@desc       创建接收线程，处理接收到的数据
 *@param:
 *@return
 *@author     GhY
 *@date       2024/07/17
 *@version    v1.0.0
 *@history:
 */
DWORD WINAPI ReceiveProcessIO(LPVOID lpParam);

/*
 *@desc       创建数据转发线程
 *@param:
 *@return
 *@author     GhY
 *@date       2024/07/24
 *@version    v1.0.0
 *@history:
 */
DWORD WINAPI TranspondMessageProcess(LPVOID lpParam);

/*
*@desc       创建客户端状态数据转发线程
*@param:
*@return
*@author     GhY
*@date       2024/07/24
*@version    v1.0.0
*@history:
*/
DWORD WINAPI ClientMessageProcess(LPVOID lpParam);

