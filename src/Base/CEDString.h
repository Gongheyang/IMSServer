/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CEDString.h
 *@描述    使用blowfish加密和解密字符串
 *
 *@作者    GhY
 *@日期    2024年8月8日
 *@版本    v1.0.0
 *
 ****************************************************/
#ifndef __CEDSTRING_H__
#define __CEDSTRING_H__
#include <string>


/*
 *@描述:    解密类
 *@作者:    GhY
 *@日期:    2024/08/08
 *@历史:
 */
class CSEncrypt
{
public:
    CSEncrypt(const char* text, const char* pwd = 0);

    ~CSEncrypt(void);

    CSEncrypt& operator= (const char* pSrc);

    operator const char* () const
    {
        return m_sEncrypted.c_str();
    }

    bool operator== (const char* text) const
    {
        return strcmp(m_sEncrypted.c_str(), text) == 0;
    }

    const char* get_crypt()
    {
        return m_sEncrypted.c_str();
    }

private:
    /*
     *@brief    加密
     *@author   GhY
     *@date     2024/08/08
     */
    static bool Encrypt(const std::string& strIn, const std::string& strPwd, std::string& strOut);

private:
    std::string m_sEncrypted;
    std::string m_sUnEncrypted;
    std::string m_sPassword;
};


/*
 *@描述:    解密类
 *@作者:    GhY
 *@日期:    2024/08/08
 *@历史:
 */
class CSDecrypt
{
public:
    CSDecrypt(const char* text, const char* pwd = 0);

    ~CSDecrypt(void);

    CSDecrypt& operator= (const char* pSrc);

    operator const char* () const
    {
        return m_sUnEncrypted.c_str();
    }

    bool operator== (const char* text) const
    {
        return strcmp(m_sUnEncrypted.c_str(), text) == 0;
    }

private:
    /*
     *@brief    解密
     *@author   GhY
     *@date     2024/08/08
     */
    static bool Decrypt(const std::string& strIn, const std::string& strPwd, std::string& strOut);

private:
    std::string m_sEncrypted;
    std::string m_sUnEncrypted;
    std::string m_sPassword;

};

#endif    //!__CEDSTRING_H__


