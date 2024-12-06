/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    get_mac_address.h
 *@描述    mac地址获取处理
 *
 *@作者    GhY
 *@日期    2024年8月7日
 *@版本    v1.0.0
 *
 ****************************************************/
#ifndef __GET_MAC_ADDRESS_H__
#define __GET_MAC_ADDRESS_H__
#include <string>


/*
 *@desc       获取当前MAC地址不包括USB类型
 *@param:     macaddr MAC地址
 *@return     true 获取成功, false 获取失败
 *@author     GhY
 *@date       2024/08/08
 *@version    v1.0.0
 *@history:
 */
bool GetMacAddress(std::string& macaddr);

bool GetMacAddressEx(std::string& macaddr);

/*
 *@desc       检查MAC地址
 *@param:     macaddr MAC地址
 *@return     true 获取成功, false 获取失败
 *@author     GhY
 *@date       2024/08/08
 *@version    v1.0.0
 *@history:
 */
bool CheckMacAddress(const std::string& macaddr);


#endif // !__GET_MAC_ADDRESS_H__
