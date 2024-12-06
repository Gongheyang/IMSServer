/****************************************************
 *
 *@Copyright (c) 2024, GhY, All rights reserved.
 *@文件    CIniConfig.cpp
 *@描述    配置文件类实现
 *
 *@作者    GhY
 *@日期    2024年7月24日
 *@版本    v1.0.0
 *
 ****************************************************/
#include "CIniConfig.h"
#include <Shlwapi.h>
#include <direct.h>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <iostream>

using namespace std;

#pragma  comment(lib, "Shlwapi.lib")


CIniConfig::CIniConfig()
    : m_filename("")
{
    m_configInfos.clear();
    std::string currentPath = GetRootPath();
    std::string filename = currentPath +  "/../config/config.ini";
    Create(filename);
}

CIniConfig::~CIniConfig() {}


CIniConfig* CIniConfig::Instance()
{
    static CIniConfig instance;
    return &instance;
}

bool CIniConfig::Init(const std::string& filename)
{
    m_filename = filename;
    return true;
}

void CIniConfig::Create(const std::string& filename, char* mode)
{
    if (filename.empty()) {
        return;
    }

    FILE* fp = fopen(filename.c_str(), mode);
    if (fp == NULL) {
        return;
    }
    fclose(fp);

    Init(filename);
    GetConfigValue();

    return ;
}

void CIniConfig::GetConfigValue()
{

    bool firstLine(false);
    text_encode eRet = check_text_encode(m_filename.c_str());
    if (eRet == eUTF8WITHBOM) {
        firstLine = true;
    }

    FILE* fp = nullptr;
    errno_t err = fopen_s(&fp, m_filename.c_str(), "r");
    if (err != 0) {
        printf_s("The file“%s”was not opened \n", m_filename.c_str());
    }
    if (fp) {
        m_configInfos.clear();
        std::string strSection = "";
        std::string strTmp = "";
        char chTmp[1024] = { 0 };
        char ch_section[256] = { 0 }, ch_key[256] = { 0 }, ch_value[256] = { 0 };     // 节， 键，值
        fseek(fp, 0, SEEK_SET);
        while (fgets(chTmp, 1024, fp) != NULL) {
            if (firstLine && (chTmp[3] == '#' || chTmp[3] == ';')) {
                // 解决utf-8-bom 文件头包含默认三位字符问题
                firstLine = false;
                continue;
            }
            if (firstLine && ((nullptr != strchr(chTmp, '//')) || (nullptr != strchr(chTmp, '/*')))) {
                // 解决utf-8-bom 文件头包含默认三位字符问题
                firstLine = false;
                continue;
            }
            // 注释行
            if (chTmp[0] == '#' || chTmp[0] == ';') {
                continue;
            }
            // 注释行
            if ((0 == strncmp("//", chTmp, 2)) || (0 == strncmp("/*", chTmp, 2))) {
                continue;
            }
            if (chTmp[strlen(chTmp) - 1] == '\n') { // 删除换行符
                chTmp[strlen(chTmp) - 1] = 0;
            }
            if (firstLine && chTmp[3] == '[' && chTmp[strlen(chTmp) - 1] == ']') {
                strSection = chTmp; // 节
                // 去除中括号 "[]"
                strSection.replace(0, 4, "");
                strSection.replace((strSection.size() - 1), 1, "");
                firstLine = false;
                continue;
            }
            if (chTmp[0] == '[' && chTmp[strlen(chTmp) - 1] == ']') {
                strSection = chTmp; // 节
                // 去除中括号 "[]"
                strSection.replace(0, 1, "");
                strSection.replace((strSection.size() - 1), 1, "");
                continue;
            }
            ConfigValue vv;
            vv.section = strSection;
            char* nextTmp = nullptr;
            char* pt = strtok_s(chTmp, "=", &nextTmp);
            while (pt != nullptr && (strlen(nextTmp) > 0)) {
                strcpy_s(ch_key, chTmp);
                strcpy_s(ch_value, nextTmp);
                vv.key = ch_key;
                vv.value = ch_value;
                m_configInfos.push_back(vv);
                break;
            }
        }
        fclose(fp);
        fp = nullptr;
    }
}

CIniConfig::text_encode CIniConfig::check_text_encode(const std::string& file_name)
{
    ifstream file_in(file_name, ios::binary);
    if (!file_in.is_open()) {
        std::cout << "打开文件失败" << endl;;
        system("pause");
        return eUNKNOW;
    }
    int head;
    unsigned char ch;
    file_in.read((char*)&ch, sizeof(ch));
    head = ch << 8;
    file_in.read((char*)&ch, sizeof(ch));
    head |= ch;
    file_in.close();
    text_encode result_code;
    switch (head) {
    case 0xFFFE:    // 65534
        result_code = eUNICODE;
        break;
    case 0xFEFF:    // 65279
        result_code = eUNICODE_BIG_ENDIAN;
        break;
    case 0xEFBB:    // 61371
        result_code = eUTF8WITHBOM;
        break;
    default:
        if (check_utf8_without_bom(file_name)) {
            result_code = eUTF8;
        } else {
            result_code = eANSI;
        }
        break;
    }
    return result_code;
}

bool CIniConfig::check_utf8_without_bom(const std::string& file_name)
{
    ifstream file_in;
    //判断文件编码
    file_in.open(file_name, ios::in);
    if (!file_in.is_open()) {
        cout << "打开文件失败" << endl;;
        system("pause");
        return false;
    }
    stringstream buffer;
    buffer << file_in.rdbuf();
    file_in.close();
    string text = buffer.str();
    int len = text.size();
    int n = 0;
    unsigned char ch;
    bool b_all_ascii = true;
    for (size_t i = 0; i < len; ++i) {
        ch = text[i];
        if ((ch & 0x80) != 0) {
            b_all_ascii = false;
        }
        if (n == 0) {
            if (ch >= 0x80) {
                if (ch >= 0xFC && ch <= 0xFD) {
                    n = 6;
                } else if (ch >= 0xF8) {
                    n = 5;
                } else if (ch >= 0xF0) {
                    n = 4;
                } else if (ch >= 0xE0) {
                    n = 3;
                } else if (ch >= 0xC0) {
                    n = 2;
                } else {
                    return false;
                }
                n--;
            }
        } else {
            if ((ch & 0xC0) != 0x80) {
                return false;
            }
            n--;
        }
    }
    if (n > 0) {
        return false;
    }
    if (b_all_ascii) {
        return false;
    }
    return true;
}

std::wstring CIniConfig::ANSIToUnicode(const std::string& str)
{
    wstring str_w;
#ifdef  _MSC_VER    // windows
    int len = str.size();
    int unicode_len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar_t* unicode_p = new wchar_t[unicode_len + 1];
    memset(unicode_p, 0, (unicode_len) * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (LPWSTR)unicode_p, unicode_len);
    str_w = (wchar_t*)unicode_p;
    delete unicode_p;
#else
    //TODO:
#endif
    return str_w;
}

void CIniConfig::save_as_utf8(std::string file_name, std::string content)
{
    wstring content_unicode = ANSIToUnicode(content);
    wofstream ofs(file_name, ios::ate);
    //std::generate_header表示带BOM的UTF-8，std::little_endian表示不带BOM的UTF-8
    //ofs.imbue(std::locale(ofs.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::generate_header>));
    ofs.imbue(std::locale(ofs.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::little_endian>));
    ofs << content_unicode;
    ofs.close();
}

std::string CIniConfig::getConfigValueWithKey(std::string strSection, std::string strkey)
{
    std::string strValue = "";
    for (auto iter = m_configInfos.begin(); iter != m_configInfos.end(); iter++) {
        if ((iter->section.compare(strSection) == 0) && (iter->key.compare(strkey) == 0)) {
            strValue = iter->value;
            break;
        }
    }
    return strValue;
}


bool CIniConfig::SetConfigValue(const std::string& section, const std::string& name, const std::string& value)
{
    if (name.empty()) {
        return false;
    }

    std::string lowerStringSection = section;
    std::string lowerStringName = name;

    std::transform(lowerStringSection.begin(), lowerStringSection.end(), lowerStringSection.begin(), ::tolower);
    std::transform(lowerStringName.begin(), lowerStringName.end(), lowerStringName.begin(), ::tolower);

    BOOL result = FALSE;
    //先判断文件是否存在
    if (PathFileExistsA(m_filename.c_str())) {
        result = WritePrivateProfileStringA(lowerStringSection.c_str(), lowerStringName.c_str(), value.c_str(), m_filename.c_str());
    }

    if (!result) {
        printf("Write config file failed: %s,last error:%d", m_filename.c_str(), GetLastError());
        return false;
    } else {
        GetConfigValue();
    }

    return true;
}

std::string CIniConfig::GetRootPath()
{
    std::string strRootPath = "";
    char* buffer = nullptr;
    if ((buffer = _getcwd(NULL, 0)) == NULL) {
        printf_s(" _getcwd function get current working directory failed! \n");
    } else {
        strRootPath = buffer;
        free(buffer);
    }
    for (int i = 0; i < strRootPath.size(); i++) {
        if (strRootPath[i] == '\\') {
            strRootPath[i] = '/';
        }
    }
    return strRootPath;
}

