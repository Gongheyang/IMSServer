/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    get_mac_address.h
 *@����    mac��ַ��ȡ����
 *
 *@����    GhY
 *@����    2024��8��7��
 *@�汾    v1.0.0
 *
 ****************************************************/
#ifndef __GET_MAC_ADDRESS_H__
#define __GET_MAC_ADDRESS_H__
#include <string>


/*
 *@desc       ��ȡ��ǰMAC��ַ������USB����
 *@param:     macaddr MAC��ַ
 *@return     true ��ȡ�ɹ�, false ��ȡʧ��
 *@author     GhY
 *@date       2024/08/08
 *@version    v1.0.0
 *@history:
 */
bool GetMacAddress(std::string& macaddr);

bool GetMacAddressEx(std::string& macaddr);

/*
 *@desc       ���MAC��ַ
 *@param:     macaddr MAC��ַ
 *@return     true ��ȡ�ɹ�, false ��ȡʧ��
 *@author     GhY
 *@date       2024/08/08
 *@version    v1.0.0
 *@history:
 */
bool CheckMacAddress(const std::string& macaddr);


#endif // !__GET_MAC_ADDRESS_H__
