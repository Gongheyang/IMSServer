/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    Singleton_T.h
 *@描述    单体模式
 *
 *@作者    GhY
 *@日期    2024年8月18日
 *@版本    v1.0.0
 *
 ****************************************************/
#ifndef __SINGLETON_T_H__
#define __SINGLETON_T_H__
#pragma once
#include <atomic>
#include "base/fylock.h"

/*
 *@描述:    定义单例模板类，解决双重检查锁定问题
 *@作者:    GhY
 *@日期:    2024/08/18
 *@历史:
 */
template<typename T>
class CSingleton_T
{
public:
    static T& Instance()
    {
        T* tmp = m_pInstance.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            CAutoLockEx<CMutexLock> lock(m_instanceMutex);
            if (tmp == nullptr) {
                static T temp;
                tmp = &temp;
                m_pInstance.store(tmp, std::memory_order_release);
            }
        }
        return *tmp;
    }

private:
    static std::atomic<T*> m_pInstance;
    static CMutexLock m_instanceMutex; // 不使用c++11的锁，否者会引入一堆东西导致编译不过
};

template<typename T> std::atomic<T*> CSingleton_T<T>::m_pInstance;

template<typename T> CMutexLock CSingleton_T<T>::m_instanceMutex;

#define SINGLETON(T) CSingleton_T<T>::Instance()

#endif //__SINGLETON_T_H__