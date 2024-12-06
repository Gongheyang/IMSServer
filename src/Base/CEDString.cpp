/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CEDString.cpp
 *@描述    使用blowfish加密和解密字符串
 *
 *@作者    GhY
 *@日期    2024年8月8日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "CEDString.h"
#include "blowfish.h"
#include "base64.h"


const static char s_cDefaultPassword[] = "%^$^JXY94aedfbs4b81nk17YGH";  // 默认加密密钥

/************************************************************************/
/*                              加密                                    */
/************************************************************************/

CSEncrypt::CSEncrypt(const char* text, const char* pwd)
    : m_sUnEncrypted(text ? text : "")
    , m_sPassword(pwd ? pwd : s_cDefaultPassword)
{
    Encrypt(m_sUnEncrypted, m_sPassword, m_sEncrypted);
}

CSEncrypt::~CSEncrypt(void)
{

}

CSEncrypt& CSEncrypt::operator=(const char* pSrc)
{
    m_sUnEncrypted = (pSrc ? pSrc : "");
    Encrypt(m_sUnEncrypted, m_sPassword, m_sEncrypted);
    return *this;
}

bool CSEncrypt::Encrypt(const std::string& strIn, const std::string& strPwd, std::string& strOut)
{
    strOut = "";
    std::string szRound = strIn;
    szRound.resize((strIn.size() + 7) / 8 * 8, '\0');
    //1. 加密
    CBlowfish bf(
        reinterpret_cast<const unsigned char* >(strPwd.c_str()),
        (unsigned long)strPwd.size());

    //szOut.reserve(szRound.size());
    char* out = 0;
    char* buf = static_cast<char*>(malloc(szRound.size()));
    if (buf == 0) {
        return false;
    }

    bool bOK = false;
    do {
        if (0 != bf.EnCode(szRound.c_str(), buf, (unsigned long)szRound.size())) {
            break;
        }
        //2.base64
        size_t len = base64_encode_alloc(buf, szRound.size(), &out);

        if (out == 0) {
            break;
        }
        strOut.append(out, len);
        bOK = true;

    } while (0);

    if (buf) {
        free(buf);
    }
    if (out) {
        free(out);
    }

    return bOK;
}


/************************************************************************/
/*                               解密                                   */
/************************************************************************/

CSDecrypt::CSDecrypt(const char* text, const char* pwd)
    : m_sEncrypted(text ? text : "")
    , m_sPassword(pwd ? pwd : s_cDefaultPassword)
{
    Decrypt(m_sEncrypted, m_sPassword, m_sUnEncrypted);
}

CSDecrypt::~CSDecrypt(void)
{

}

CSDecrypt& CSDecrypt::operator=(const char* pSrc)
{
    m_sEncrypted = (pSrc ? pSrc : "");
    Decrypt(m_sEncrypted, m_sPassword, m_sUnEncrypted);
    return *this;
}

bool CSDecrypt::Decrypt(const std::string& strIn, const std::string& strPwd, std::string& strOut)
{
    strOut = "";
    std::string szRound = strIn;
    szRound.resize((strIn.size() + 7) / 8 * 8, '\0');
    //1. base64
    char*   dec64 = 0;
    char*   decbf = 0;
    size_t dec64_len = 0;
    bool bOK = false;

    do {
        if (!base64_decode_alloc(strIn.c_str(), strIn.size(), &dec64, &dec64_len)) {
            break;
        }
        if (dec64_len % 8 != 0) {
            return false;
        }

        CBlowfish bf(
            reinterpret_cast<const unsigned char* >(strPwd.c_str()),
            (unsigned long)strPwd.size());

        decbf = static_cast<char*>(malloc(dec64_len)) ;
        if (decbf == 0) {
            break;
        }
        if (0 != bf.DeCode(dec64, decbf, (unsigned long)dec64_len)) {
            break;
        }
        strOut.append(decbf, dec64_len);
    } while (0);

    if (dec64) {
        free(dec64);
    }
    if (decbf) {
        free(decbf);
    }

    return bOK;
}
