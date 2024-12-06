/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@�ļ�    CIniConfig.h
 *@����    �����ļ�������
 *
 *@����    GhY
 *@����    2024��7��24��
 *@�汾    v1.0.0
 *
 ****************************************************/
#ifndef __CINICONFIG_H__
#define __CINICONFIG_H__
#include <string>
#include <vector>
#include <algorithm>

/*
 *@brief    �����ļ��洢�ṹ
 *@author   GhY
 *@date     2024/07/24
 */
struct ConfigValue {
    std::string section;
    std::string key;
    std::string value;

    ConfigValue()
    {
        section = "";
        key = "";
        value = "";
    }
};


/*
 *@����:    ��ȡ�����ļ���
 *@����:    GhY
 *@����:    2024/07/24
 *@��ʷ:
 */
class CIniConfig
{
public:

    enum text_encode {
        eUNKNOW = 0,
        eUNICODE,
        eUNICODE_BIG_ENDIAN,
        eUTF8WITHOUTBOM,
        eUTF8WITHBOM,
        eUTF8,
        eANSI,
    };

    CIniConfig();

    ~CIniConfig();

    /*
     *@brief    �����ӿ�
     *@author   GhY
     *@date     2024/07/24
     */
    static CIniConfig* Instance();


    /*
     *@desc       ��ȡ��ǰ�������·��
     *@return     ��ǰ�������·��
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    std::string GetRootPath();

    /*
     *@desc       ��ʼ��
     *@param:     �ļ�ȫ·����
     *@return
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    bool Init(const std::string& filename);

    /*
     *@desc       �����ļ��������򴴽�
     *@param:     filename �ļ�ȫ·����
     *@param:     mode ��ģʽ
     *@return
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    void Create(const std::string& filename, char* mode = "a+");

    /*
     *@desc       ���ݲ�����ȡ����
     *@param:     strSection   ��
     *@param:     strkey      ��
     *@return     value     ֵ
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
    */
    std::string getConfigValueWithKey(std::string strSection, std::string strkey);

    /*
     *@desc       ���ݲ�����������
     *@param:     section   ��
     *@param:     name      ��
     *@param:     value     ֵ
     *@return     true ����ɹ���false ����ʧ��
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    bool SetConfigValue(const std::string& section, const std::string& name, const std::string& value);

private:
    /*
     *@brief    ��ȡ��������
     *@author   GhY
     *@date     2024/07/24
     */
    void GetConfigValue();

    text_encode check_text_encode(const std::string& file_name);

    bool check_utf8_without_bom(const std::string& file_name);

    std::wstring ANSIToUnicode(const std::string& str);

    void save_as_utf8(std::string file_name, std::string content);

private:
    std::string m_filename;      // �����ļ�ȫ·��

    std::vector<ConfigValue> m_configInfos;
};


#define g_ConfigPtr (*(CIniConfig::Instance()))

#endif //!__CINICONFIG_H__


