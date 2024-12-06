/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    WinPublicIO.h
 *@����    ��ɷ����� ���߳���Ϣ����
 *
 *@����    GhY
 *@����    2024��7��17��
 *@�汾    v1.0.0
 *
 ****************************************************/
#include "PublicDefine.h"
#include"windows.h"
#include <map>


/*
 *@desc       ���������̣߳�������յ�������
 *@param:
 *@return
 *@author     GhY
 *@date       2024/07/17
 *@version    v1.0.0
 *@history:
 */
DWORD WINAPI ReceiveProcessIO(LPVOID lpParam);

/*
 *@desc       ��������ת���߳�
 *@param:
 *@return
 *@author     GhY
 *@date       2024/07/24
 *@version    v1.0.0
 *@history:
 */
DWORD WINAPI TranspondMessageProcess(LPVOID lpParam);

/*
*@desc       �����ͻ���״̬����ת���߳�
*@param:
*@return
*@author     GhY
*@date       2024/07/24
*@version    v1.0.0
*@history:
*/
DWORD WINAPI ClientMessageProcess(LPVOID lpParam);

