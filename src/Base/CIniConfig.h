/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CIniConfig.h
 *@描述    配置文件类声明
 *
 *@作者    GhY
 *@日期    2024年7月24日
 *@版本    v1.0.0
 *
 ****************************************************/
#ifndef __CINICONFIG_H__
#define __CINICONFIG_H__
#include <string>
#include <vector>
#include <algorithm>

/*
 *@brief    配置文件存储结构
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
 *@描述:    读取配置文件类
 *@作者:    GhY
 *@日期:    2024/07/24
 *@历史:
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
     *@brief    单例接口
     *@author   GhY
     *@date     2024/07/24
     */
    static CIniConfig* Instance();


    /*
     *@desc       获取当前软件运行路径
     *@return     当前软件运行路径
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    std::string GetRootPath();

    /*
     *@desc       初始化
     *@param:     文件全路径名
     *@return
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    bool Init(const std::string& filename);

    /*
     *@desc       配置文件不存在则创建
     *@param:     filename 文件全路径名
     *@param:     mode 打开模式
     *@return
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    void Create(const std::string& filename, char* mode = "a+");

    /*
     *@desc       根据参数获取配置
     *@param:     strSection   项
     *@param:     strkey      键
     *@return     value     值
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
    */
    std::string getConfigValueWithKey(std::string strSection, std::string strkey);

    /*
     *@desc       根据参数保存配置
     *@param:     section   项
     *@param:     name      键
     *@param:     value     值
     *@return     true 保存成功；false 保存失败
     *@author     GhY
     *@date       2024/07/24
     *@version    v1.0.0
     *@history:
     */
    bool SetConfigValue(const std::string& section, const std::string& name, const std::string& value);

private:
    /*
     *@brief    获取所有配置
     *@author   GhY
     *@date     2024/07/24
     */
    void GetConfigValue();

    text_encode check_text_encode(const std::string& file_name);

    bool check_utf8_without_bom(const std::string& file_name);

    std::wstring ANSIToUnicode(const std::string& str);

    void save_as_utf8(std::string file_name, std::string content);

private:
    std::string m_filename;      // 配置文件全路径

    std::vector<ConfigValue> m_configInfos;
};


#define g_ConfigPtr (*(CIniConfig::Instance()))

#endif //!__CINICONFIG_H__


