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
#include "get_mac_address.h"


#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <algorithm>
#include <string>
#include <list>

#ifdef WIN32
#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wbemcli.h>
#include <Strsafe.h>
#include <ntddndis.h>
#include <winioctl.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlconv.h>
#include <atlcomcli.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>


#define HEAP_MALLOC(x)  HeapAlloc(GetProcessHeap(), 0, (x))
#define HEAP_FREE(x)    HeapFree(GetProcessHeap(), 0, (x))

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Wbemuuid.lib")

#define MACADDRESS_BYTELEN      6   // MAC地址字节长度

typedef struct _T_MACADDRESS {
    BYTE  PermanentAddress[MACADDRESS_BYTELEN];   // 原生MAC地址
    BYTE  MACAddress[MACADDRESS_BYTELEN];         // 当前MAC地址
} T_MACADDRESS;

typedef struct _T_WQL_QUERY {
    CHAR* szSelect;       /*!< SELECT语句 */
    WCHAR*    szProperty; /*!< 属性字段 */
} T_WQL_QUERY;

/**
 *@brief WQL QUERY with net cards
 */
const T_WQL_QUERY szWQLQuery[] = {
    /** Include usb type */
    "SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",
    L"PNPDeviceID",

    /** Not include usb type */
    "SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%')) AND (NOT (PNPDeviceID LIKE 'USB%'))",
    L"PNPDeviceID"
};


//-------------------导出函数-------------

/** \brief 结合WMI和DeviceIoControl获取网卡原生MAC地址和当前MAC地址
 * \param iQueryType - 需要获取的网卡类型  0：包括USB网卡 1：不包括USB网卡
 * \param pMacAddress - 存储网卡MAC地址
 * \param uSize - 可存储的最大网卡数目
 * \return -1：不支持的设备属性值;  -2：WMI连接失败;  -3：不正确的WQL查询语句;  >=0：获取的网卡数目
 */
INT WMI_MacAddress(INT iQueryType, T_MACADDRESS* pMacAddress, INT iSize);

std::string WMI_MacAddress(INT iQueryType);

/*
 *@brief    获取默认接口IP
 *@author   GhY
 *@date     2024/08/08
 */
bool get_default_interface_ip(std::string& ip, std::string& gateway);

bool get_output_ip(const std::string& test_ip, std::string& output_ip);

static bool test_connection(const SOCKADDR_IN& localaddr,
                            const SOCKADDR_IN& serveraddr, unsigned long timeout, std::string& localip);

bool test_connection(const char* local,
                     const char* server, int port, std::string& localip);

/*
 *@brief    检测本地IP
 *@author   GhY
 *@date     2024/08/08
 */
bool DetectLocalIP(std::string& szAddr, const std::string& testServer = "www.baidu.com", unsigned port = 80);

static BOOL WMI_DoWithPNPDeviceID(const TCHAR* PNPDeviceID, T_MACADDRESS* pMacAddress, INT iIndex)
{
    TCHAR DevicePath[MAX_PATH];
    HANDLE    hDeviceFile;
    BOOL  isOK = FALSE;

    // 生成设备路径名
    StringCchCopy(DevicePath, MAX_PATH, TEXT("\\\\.\\"));
    StringCchCat(DevicePath, MAX_PATH, PNPDeviceID);
    StringCchCat(DevicePath, MAX_PATH, TEXT("#{ad498944-762f-11d0-8dcb-00c04fc3358c}"));

    // 将“PNPDeviceID”中的“/”替换成“#”，以获得真正的设备路径名
    std::replace(DevicePath + 4, DevicePath + 4 + _tcslen(PNPDeviceID), TEXT('\\'), TEXT('#'));

    // 获取设备句柄
    hDeviceFile = CreateFile(DevicePath,
                             0,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

    if (hDeviceFile != INVALID_HANDLE_VALUE) {
        ULONG   dwID;
        BYTE    ucData[8];
        DWORD   dwByteRet;

        // 获取当前MAC地址
        dwID = OID_802_3_CURRENT_ADDRESS;
        isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
        if (isOK) {
            memcpy(pMacAddress[iIndex].MACAddress, ucData, dwByteRet);
            // 获取原生MAC地址
            dwID = OID_802_3_PERMANENT_ADDRESS;
            isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
            if (isOK) {
                memcpy(pMacAddress[iIndex].PermanentAddress, ucData, dwByteRet);
            }
        }

        CloseHandle(hDeviceFile);
    }

    return isOK;
}

INT WMI_MacAddress(INT iQueryType, T_MACADDRESS* pMacAddress, INT iSize)
{
    HRESULT hres;
    INT   iTotal = 0;

    // 判断查询类型是否支持
    if ((iQueryType < 0) || (iQueryType >= sizeof(szWQLQuery) / sizeof(T_WQL_QUERY))) {
        return -1;    // 查询类型不支持
    }

    // 初始化COM
    hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hres) && (RPC_E_CHANGED_MODE != hres)) {
        return -2;
    }

    // 设置COM的安全认证级别
    hres = CoInitializeSecurity(
               NULL,
               -1,
               NULL,
               NULL,
               RPC_C_AUTHN_LEVEL_DEFAULT,
               RPC_C_IMP_LEVEL_IMPERSONATE,
               NULL,
               EOAC_NONE,
               NULL
           );
    if (FAILED(hres)) {
        if (hres != RPC_E_TOO_LATE) {
            CoUninitialize();
            return -2;
        }
    }

    // 获得WMI连接COM接口
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hres)) {
        CoUninitialize();
        return -2;
    }

    // 通过连接接口连接WMI的内核对象名"ROOT//CIMV2"
    CComPtr<IWbemServices> pSvc;
    hres = pLoc->ConnectServer(
               CComBSTR("ROOT\\CIMV2"),
               NULL,
               NULL,
               NULL,
               0,
               NULL,
               NULL,
               &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return -2;
    }

    // 设置请求代理的安全级别
    hres = CoSetProxyBlanket(
               pSvc,
               RPC_C_AUTHN_WINNT,
               RPC_C_AUTHZ_NONE,
               NULL,
               RPC_C_AUTHN_LEVEL_CALL,
               RPC_C_IMP_LEVEL_IMPERSONATE,
               NULL,
               EOAC_NONE
           );
    if (FAILED(hres)) {
        pSvc.Release();
        pLoc->Release();
        CoUninitialize();
        return -2;
    }

    // 通过请求代理来向WMI发送请求
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
               CComBSTR("WQL"),
               CComBSTR(szWQLQuery[iQueryType].szSelect),
               WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
               NULL,
               &pEnumerator
           );
    if (FAILED(hres)) {
        pSvc.Release();
        pLoc->Release();
        CoUninitialize();
        return -3;
    }

    // 循环枚举所有的结果对象
    while (pEnumerator) {
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        if ((pMacAddress != NULL) && (iTotal >= iSize)) {
            break;
        }

        pEnumerator->Next(
            WBEM_INFINITE,
            1,
            &pclsObj,
            &uReturn
        );

        if (uReturn == 0) {
            break;
        }

        VARIANT vtProperty;
        TCHAR szProperty[128];

        // 获取网卡设备标识符
        VariantInit(&vtProperty);
        pclsObj->Get(szWQLQuery[iQueryType].szProperty, 0, &vtProperty, NULL, NULL);
        USES_CONVERSION;
        StringCchCopy(szProperty, sizeof(szProperty) / sizeof(TCHAR), W2T(vtProperty.bstrVal));
        VariantClear(&vtProperty);

        if (pMacAddress != NULL) {
            // 通过设备标识符获取原生MAC地址和当前MAC地址
            if (WMI_DoWithPNPDeviceID(szProperty, pMacAddress, iTotal)) {
                iTotal++;
            }
        } else {
            iTotal++;
        }

        pclsObj->Release();
    } // End While

    // 释放资源
    pEnumerator->Release();
    pSvc.Release();
    pLoc->Release();
    CoUninitialize();

    return iTotal;
}


std::string WMI_MacAddress(INT iQueryType)
{
    HRESULT hres;
    INT iTotal = 0;
    std::string strMac;

    // 判断查询类型是否支持
    if ((iQueryType < 0) || (iQueryType >= sizeof(szWQLQuery) / sizeof(T_WQL_QUERY))) {
        return "";    // 查询类型不支持
    }

    // 初始化COM
    hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hres) && (RPC_E_CHANGED_MODE != hres)) {
        return "";
    }

    // 设置COM的安全认证级别
    hres = CoInitializeSecurity(
               NULL,
               -1,
               NULL,
               NULL,
               RPC_C_AUTHN_LEVEL_DEFAULT,
               RPC_C_IMP_LEVEL_IMPERSONATE,
               NULL,
               EOAC_NONE,
               NULL
           );
    if (FAILED(hres)) {
        if (hres != RPC_E_TOO_LATE) {
            CoUninitialize();
            return "";
        }
    }

    // 获得WMI连接COM接口
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            reinterpret_cast<LPVOID*>(&pLoc));
    if (FAILED(hres)) {
        CoUninitialize();
        return "";
    }

    // 通过连接接口连接WMI的内核对象名"ROOT//CIMV2"
    CComPtr<IWbemServices> pSvc;
    hres = pLoc->ConnectServer(
               CComBSTR("ROOT\\CIMV2"),
               NULL,
               NULL,
               NULL,
               0,
               NULL,
               NULL,
               &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // 设置请求代理的安全级别
    hres = CoSetProxyBlanket(
               pSvc,
               RPC_C_AUTHN_WINNT,
               RPC_C_AUTHZ_NONE,
               NULL,
               RPC_C_AUTHN_LEVEL_CALL,
               RPC_C_IMP_LEVEL_IMPERSONATE,
               NULL,
               EOAC_NONE
           );
    if (FAILED(hres)) {
        pSvc.Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // 通过请求代理来向WMI发送请求
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
               CComBSTR("WQL"),
               CComBSTR(szWQLQuery[iQueryType].szSelect),
               WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
               NULL,
               &pEnumerator
           );
    if (FAILED(hres)) {
        pSvc.Release();
        pLoc->Release();
        CoUninitialize();
        return "";
    }

    // 循环枚举所有的结果对象
    while (pEnumerator) {
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (uReturn == 0) {
            break;
        }

        VARIANT vtProperty;
        TCHAR szProperty[128];

        // 获取网卡设备标识符
        VariantInit(&vtProperty);
        pclsObj->Get(szWQLQuery[iQueryType].szProperty, 0, &vtProperty, NULL, NULL);
        USES_CONVERSION;
        StringCchCopy(szProperty, sizeof(szProperty) / sizeof(TCHAR), W2T(vtProperty.bstrVal));
        VariantClear(&vtProperty);

        T_MACADDRESS macAddress = { 0 };
        // 通过设备标识符获取原生MAC地址和当前MAC地址
        if (WMI_DoWithPNPDeviceID(szProperty, &macAddress, iTotal)) {
            char mac_str[18] = { 0 };
            const unsigned char* addr = (unsigned char*)&macAddress.MACAddress[0];
            _snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
            mac_str[17] = 0;
            strMac.append(mac_str, strlen(mac_str));
            strMac.append(",");
        }

        pclsObj->Release();
    } // End While

    // 释放资源
    pEnumerator->Release();
    pSvc.Release();
    pLoc->Release();
    CoUninitialize();

    if (strMac.length() > 0) {
        strMac.at(strMac.length() - 1) = '\0';
    }
    return strMac;
}


std::string get_mac_adress(const std::string& localip = "")
{
    std::string mac_adress;

    ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO* pAdapterInfoBkp = NULL, *pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW) {
        delete[] pAdapterInfo;
        pAdapterInfo = (IP_ADAPTER_INFO*)new char[ulAdapterInfoSize];
        pAdapterInfoBkp = pAdapterInfo;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS) {
        do {
            // 遍历所有适配器
            //MAC地址
            std::string mac;
            for (UINT i = 0; i < pAdapterInfo->AddressLength; i++) {
                char szTmp[8] = { 0 };
                sprintf(szTmp, "%02X%c", pAdapterInfo->Address[i], (i == pAdapterInfo->AddressLength - 1) ? '\0' : ':');
                mac.append(szTmp);
            }
            if (localip.empty()) {
                if (!mac.empty() && !mac_adress.empty()) {
                    mac_adress += ";";
                }
                mac_adress += mac;
            }
            //IP地址
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING* pIpAddrString = &(pAdapterInfo->IpAddressList);
            do {
                std::string address_ip = pIpAddrString->IpAddress.String;
                if (address_ip == localip) {
                    mac_adress = mac;
                    break;
                }
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);

            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    } else {
        printf("[mac] GetAdaptersInfo last error:%d \n", GetLastError());
    }
    if (pAdapterInfoBkp) {
        delete[] pAdapterInfoBkp;
    }
    return mac_adress;
}


#ifdef _UNICODE

bool GetMacAddress(std::string& mac)
{
    std::string localaddr;
    bool result = DetectLocalIP(localaddr);
    if (result) {
        printf("GetMacAddress local ip:%s \n", localaddr.c_str());
        mac = get_mac_adress(localaddr);
        printf("GetMacAddress mac address:%s \n", mac.c_str());
    } else {
        printf("GetMacAddress get localip failed error:%d", GetLastError());
    }
    if (mac.empty()) {
        printf("GetMacAddress mac address is empty,get mac address with wmi");
        T_MACADDRESS mac_address = { 0 };
        int ret = WMI_MacAddress(1, &mac_address, 1);
        if (ret >= 0) {
            char mac_str[18] = { 0 };
            const unsigned char* addr = (unsigned char*)&mac_address.MACAddress[0];
            _snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
            mac_str[17] = 0;
            mac.append(mac_str, strlen(mac_str));
            result = true;
        } else {
            result = false;
        }
    }
    if (mac.empty()) {
        result = false;
    }
    return result;
}

bool GetMacAddressEx(std::string& macaddr)
{
    macaddr = WMI_MacAddress(1);
    if (macaddr.length() > 0) {
        return true;
    } else {
        macaddr = get_mac_adress();
        //
        for (size_t n = 0; n < macaddr.length(); n++) {
            if (macaddr.at(n) == ';') {
                macaddr.at(n) = ',';
            }
        }
    }

    return true;
}

bool CheckMacAddress(const std::string& macaddr)
{
    std::string all_mac_string = get_mac_adress();
    if (all_mac_string.empty()) {
        printf(" [%s] [%d] get all mac address failed:%d \n", __FUNCTION__, __LINE__, GetLastError());
        return false;
    }
    std::string szTemp = all_mac_string;
    std::transform(all_mac_string.begin(), all_mac_string.end(), szTemp.begin(), ::tolower);
    all_mac_string = szTemp;
    szTemp = macaddr;
    std::transform(macaddr.begin(), macaddr.end(), szTemp.begin(), ::tolower);
    std::string mac = szTemp;
    std::string::size_type find_size = all_mac_string.find(mac);
    if (find_size != std::string::npos) {
        return true;
    }
    return false;
}

#else

bool get_default_interface(char* ip, char* gateway, IF_INDEX* ifindex)   //every param could be null, if not null, will be fill, qstring should be pre allocated
{
    PMIB_IPFORWARDTABLE pIpForwardTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    bool bDefaultRouteFound = false;

    pIpForwardTable =
        (MIB_IPFORWARDTABLE*)malloc(sizeof(MIB_IPFORWARDTABLE));
    if (pIpForwardTable == NULL) {
        printf(" [%s] [%d] Error allocating memory \n", __FUNCTION__, __LINE__);
        return false;
    }

    if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        free(pIpForwardTable);
        pIpForwardTable = (MIB_IPFORWARDTABLE*)malloc(dwSize);
        if (pIpForwardTable == NULL) {
            printf(" [%s] [%d] Error allocating memory \n", __FUNCTION__, __LINE__);
            return false;
        }
    }

    DWORD min_metric = -1;
    DWORD next_hop_gateway = 0;

    if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) {
        printf("[mac] Number of route entries: %d \n",
               (int)pIpForwardTable->dwNumEntries);
        struct in_addr IpAddr;
        //1. find 0.0.0.0 gw first
        for (int i = 0; i < (int)pIpForwardTable->dwNumEntries; i++) {
            if (pIpForwardTable->table[i].dwForwardDest == 0 //we found one 0.0.0.0
                    && pIpForwardTable->table[i].dwForwardType == MIB_IPROUTE_TYPE_INDIRECT) { //0.0.0.0 is always indirect
                if (pIpForwardTable->table[i].dwForwardMetric1 < min_metric) { //find the one have the minimum metric
                    min_metric = pIpForwardTable->table[i].dwForwardMetric1;
                    //copy values
                    next_hop_gateway = pIpForwardTable->table[i].dwForwardNextHop;

                    IpAddr.S_un.S_addr = (u_long)pIpForwardTable->table[i].dwForwardNextHop;
                    if (gateway) {
                        strcpy(gateway, inet_ntoa(IpAddr));
                    }
                }
            }
        }
        if (!next_hop_gateway) {
            free(pIpForwardTable);
            return false;
        }
        //2. find the gateway in which interface
        for (int i = 0; i < (int)pIpForwardTable->dwNumEntries; i++) {
            if (((pIpForwardTable->table[i].dwForwardMask & next_hop_gateway) == pIpForwardTable->table[i].dwForwardDest)
                    && pIpForwardTable->table[i].dwForwardType == MIB_IPROUTE_TYPE_DIRECT
                    /*&& pIpForwardTable->table[i].dwForwardProto == MIB_IPPROTO_LOCAL*/) {
                IpAddr.S_un.S_addr = (u_long)pIpForwardTable->table[i].dwForwardNextHop;
                if (ip) {
                    strcpy(ip, inet_ntoa(IpAddr));
                }
                if (ifindex) {
                    *ifindex = pIpForwardTable->table[i].dwForwardIfIndex;
                }
                bDefaultRouteFound = true;
                break;
            }
        }

        free(pIpForwardTable);
    } else {
        printf(" [%s] [%d] GetIpForwardTable failed. \n", __FUNCTION__, __LINE__);
        free(pIpForwardTable);
        return false;
    }
    return bDefaultRouteFound;
}

bool GetMacAddress(std::string& mac)
{
    ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO* pAdapterInfoBkp = NULL, *pAdapterInfo = NULL;
    bool bFound = false;
    IF_INDEX ifindex;
    if (!get_default_interface(NULL, NULL, &ifindex)) {
        return false;
    }
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulAdapterInfoSize);
    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW) { // 缓冲区不够大
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulAdapterInfoSize);
        pAdapterInfoBkp = pAdapterInfo;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS) {
        do { // 遍历所有适配器
            if (pAdapterInfo->Index == ifindex && pAdapterInfo->AddressLength == 6) {   // 判断是否为以太网接口
                char mac_str[18] = {0};
                _snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                          pAdapterInfo->Address[0],
                          pAdapterInfo->Address[1],
                          pAdapterInfo->Address[2],
                          pAdapterInfo->Address[3],
                          pAdapterInfo->Address[4],
                          pAdapterInfo->Address[5]
                         );
                mac = std::string(mac_str, strlen(mac_str));
                bFound = true;
                break;
            }
            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    }

    if (pAdapterInfoBkp) {
        free(pAdapterInfoBkp);
    }

    return bFound;
}

bool GetMacAddressEx(std::string& macaddr)
{
    ULONG ulAdapterInfoSize = sizeof(IP_ADAPTER_INFO);
    IP_ADAPTER_INFO* pAdapterInfoBkp = NULL, *pAdapterInfo = NULL;
    bool bFound = false;
    IF_INDEX ifindex;
    if (!get_default_interface(NULL, NULL, &ifindex)) {
        return false;
    }
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulAdapterInfoSize);
    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_BUFFER_OVERFLOW) { // 缓冲区不够大
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulAdapterInfoSize);
        pAdapterInfoBkp = pAdapterInfo;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulAdapterInfoSize) == ERROR_SUCCESS) {
        do { // 遍历所有适配器
            if (pAdapterInfo->Index == ifindex && pAdapterInfo->AddressLength == 6) {  // 判断是否为以太网接口
                char mac_str[18] = { 0 };
                _snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                          pAdapterInfo->Address[0],
                          pAdapterInfo->Address[1],
                          pAdapterInfo->Address[2],
                          pAdapterInfo->Address[3],
                          pAdapterInfo->Address[4],
                          pAdapterInfo->Address[5]
                         );
                std::string mac = std::string(mac_str, strlen(mac_str));

                macaddr.append(mac);
                macaddr.append(",");
                bFound = true;
            }
            pAdapterInfo = pAdapterInfo->Next;
        } while (pAdapterInfo);
    }

    if (pAdapterInfoBkp) {
        free(pAdapterInfoBkp);
    }

    if (macaddr.length() > 0) {
        macaddr.at(macaddr.length() - 1) = '\0';
    }
    return bFound;
}

#endif

bool GetMacAddress2(std::string& mac, bool withwmi)
{
    if (!withwmi) {
        mac = get_mac_adress();
        return true;
    }

    T_MACADDRESS mac_address = {0};
    int result = WMI_MacAddress(1, &mac_address, 1);
    if (result >= 0) {
        char mac_str[18] = {0};
        const unsigned char* addr = (unsigned char*)&mac_address.MACAddress[0];
        _snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                  addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        mac_str[17] = 0;
        mac.append(mac_str, strlen(mac_str));
    } else {
        return false;
    }

    return true;
}


bool get_output_ip(const std::string& test_ip, std::string& output_ip)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == -1) {
        return false;
    }

    struct sockaddr_in connaddr;
    connaddr.sin_family = AF_INET;
    connaddr.sin_port = htons(1900); //端口任意
    connaddr.sin_addr.s_addr = inet_addr(test_ip.c_str());

    //利用udp套接字调用connect来确定出口
    if (connect(s, (struct sockaddr*)&connaddr, sizeof(connaddr)) == -1) {
        closesocket(s);
        return false;
    }

    //获取本地关联的接口地址
    struct sockaddr_in localaddr;
    socklen_t addr_len = sizeof(localaddr);
    if (getsockname(s, (struct sockaddr*)&localaddr, &addr_len) == -1) {
        closesocket(s);
        return false;
    }

    char ip[32] = { 0 };
    if (::inet_ntop(AF_INET, &localaddr.sin_addr, ip, sizeof(ip))) {
        closesocket(s);
        return false;
    }

    closesocket(s);
    output_ip = ip;
    return true;
}



//a function process windows route table and get default interface ip address
bool get_default_interface_ip(std::string& ip, std::string& gateway)
{
    PMIB_IPFORWARDTABLE pIpForwardTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    pIpForwardTable =
        (MIB_IPFORWARDTABLE*)HEAP_MALLOC(sizeof(MIB_IPFORWARDTABLE));
    if (pIpForwardTable == NULL) {
        printf("Error allocating memory\n");
        return false;
    }

    if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        HEAP_FREE(pIpForwardTable);
        pIpForwardTable = (MIB_IPFORWARDTABLE*)HEAP_MALLOC(dwSize);
        if (pIpForwardTable == NULL) {
            printf("Error allocating memory\n");
            return false;
        }
    }

    DWORD min_metric = -1;
    DWORD next_hop_gateway = 0;
    DWORD IfIndex = 0;

    if ((dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0)) == NO_ERROR) {
        printf("\tNumber of entries: %d\n",
               (int)pIpForwardTable->dwNumEntries);
        struct in_addr IpAddr;
        //1. find 0.0.0.0 gw first
        for (int i = 0; i < (int)pIpForwardTable->dwNumEntries; i++) {
            if (pIpForwardTable->table[i].dwForwardDest == 0 //we found one 0.0.0.0
                    && pIpForwardTable->table[i].dwForwardType == MIB_IPROUTE_TYPE_INDIRECT) { //0.0.0.0 is always indirect
                if (pIpForwardTable->table[i].dwForwardMetric1 < min_metric) { //find the one have the minimum metric
                    min_metric = pIpForwardTable->table[i].dwForwardMetric1;
                    //copy values
                    next_hop_gateway = pIpForwardTable->table[i].dwForwardNextHop;
                    IfIndex = pIpForwardTable->table[i].dwForwardIfIndex;
                    IpAddr.S_un.S_addr = (u_long)pIpForwardTable->table[i].dwForwardNextHop;
                    gateway = inet_ntoa(IpAddr);
                }
            }
        }
        if (!next_hop_gateway) {
            HEAP_FREE(pIpForwardTable);
            return false;
        }
        //2. find the gateway in which interface
        for (int i = 0; i < (int)pIpForwardTable->dwNumEntries; i++) {
            if (((pIpForwardTable->table[i].dwForwardMask & next_hop_gateway) == pIpForwardTable->table[i].dwForwardDest)
                    && pIpForwardTable->table[i].dwForwardType == MIB_IPROUTE_TYPE_DIRECT
                    && pIpForwardTable->table[i].dwForwardProto == MIB_IPPROTO_LOCAL) {
                IpAddr.S_un.S_addr = (u_long)pIpForwardTable->table[i].dwForwardNextHop;
                ip = inet_ntoa(IpAddr);
                break;
            }
        }
        //3. find from ipaddr table
        if (ip.empty() && IfIndex > 0) {
            PMIB_IPADDRTABLE pIpAddressTable = (MIB_IPADDRTABLE*)HEAP_MALLOC(sizeof(MIB_IPADDRTABLE));
            PMIB_IPADDRROW pAddrRow;
            if (pIpAddressTable == NULL) {
                printf("Error allocating memory\n");
                return false;
            }
            dwSize = 0;
            if (GetIpAddrTable(pIpAddressTable, &dwSize, TRUE) == ERROR_INSUFFICIENT_BUFFER) {
                HEAP_FREE(pIpAddressTable);
                pIpAddressTable = (MIB_IPADDRTABLE*)HEAP_MALLOC(dwSize);
                if (pIpAddressTable == NULL) {
                    printf("Error allocating memory\n");
                    return false;
                }
            }
            if ((dwRetVal = GetIpAddrTable(pIpAddressTable, &dwSize, 0)) == NO_ERROR) {
                for (DWORD i = 0; i < pIpAddressTable->dwNumEntries; i++) {
                    pAddrRow = (PMIB_IPADDRROW) & (pIpAddressTable->table[i]);
                    if (IfIndex == pAddrRow->dwIndex) {
                        IpAddr.S_un.S_addr = pAddrRow->dwAddr;
                        ip = inet_ntoa(IpAddr);
                        break;
                    }
                }
            }
            HEAP_FREE(pIpAddressTable);
        }


        HEAP_FREE(pIpForwardTable);
    } else {
        printf("\tGetIpForwardTable failed.\n");
        HEAP_FREE(pIpForwardTable);
        return false;
    }
    return true;
}

static bool DetectLocalIPByGetHostName(std::list<std::string>& ip_list, bool filter_arm)
{
    char szHostName[64] = { 0 };
    if (gethostname(szHostName, sizeof(szHostName)) != 0) {
        return false;
    }

    hostent* lphost = gethostbyname(szHostName);
    if (NULL == lphost) {
        return false;
    }

    for (int i = 0; lphost->h_addr_list[i]; i++) {
        SOCKADDR_IN sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr_list[i])->s_addr;
        // 过滤 android 中ip为127.0.0.1的地址，无法与其他机器进行UDP
        if (filter_arm) {
            if (0 != strcmp(inet_ntoa(sockAddr.sin_addr), "127.0.0.1")) {
                ip_list.push_back(inet_ntoa(sockAddr.sin_addr));
            }
        } else {
            ip_list.push_back(inet_ntoa(sockAddr.sin_addr));
        }
    }

    return true;
}

bool DetectLocalIP(std::string& szAddr, const std::string& testServer, unsigned port)
{
#ifdef WIN32
    if (get_output_ip("220.181.38.148", szAddr)) {  //任意指定一个IP,不需要能连通（此IP为baidu公网IP）
        return true;
    }

    std::string ip;
    std::string gateway;
    if (get_default_interface_ip(ip, gateway)) {
        if (!ip.empty()) {
            szAddr = ip;
            printf("get_default_interface_ip ok with %s \n", ip.c_str());
            return true;
        }
    }
#endif

    std::list<std::string> ip_list;
    bool filter_arm = false;

    if (ip_list.empty()) {
        if (!DetectLocalIPByGetHostName(ip_list, filter_arm) || ip_list.empty()) {
            return false;
        }
    }

#ifdef DEBUG
    std::list<std::string>::iterator it = ip_list.begin();
    for (; it != ip_list.end(); ++it) {
        printf("[test] ip: %s\n", it->c_str());
    }
#endif

    //尝试连接测试服务器
    if (test_connection("0.0.0.0", testServer.c_str(), port, szAddr)) {
        if (szAddr == "127.0.0.1") {
            szAddr = *(ip_list.begin());
        }

        return true;
    } else {
        if (!ip_list.empty()) {
            printf("test connect server failed, return first ip\n");
            szAddr = *(ip_list.begin());
        }
        return true;
    }

    return false;
}

static bool test_connection(const SOCKADDR_IN& localaddr,
                            const SOCKADDR_IN& serveraddr, unsigned long timeout, std::string& localip)
{
    bool bReturn = false;
    int err = -1;
    unsigned long bLock = 1;

    //socket
    SOCKET sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        return false;
    }

    //bind
    if (bind(sockfd, (SOCKADDR*)&localaddr, sizeof(localaddr)) == SOCKET_ERROR) {
        if (sockfd != INVALID_SOCKET) {
            closesocket(sockfd);
        }
        return false;
    }

    //ioctrl
    if (ioctlsocket(sockfd, FIONBIO, &bLock) == SOCKET_ERROR) {
        if (sockfd != INVALID_SOCKET) {
            closesocket(sockfd);
        }
        return false;
    }

    //connect
    err = connect(sockfd, (sockaddr*)&serveraddr, sizeof(serveraddr));
#ifdef WIN32
    if (SOCKET_ERROR == err && GetLastError() != WSAEWOULDBLOCK)
#else
    if (SOCKET_ERROR == err && errno != EINPROGRESS)
#endif
    {
        if (sockfd != INVALID_SOCKET) {
            closesocket(sockfd);
        }
        return false;
    } else if (SOCKET_ERROR == err) {
        fd_set setWrite;
        FD_ZERO(&setWrite);
        FD_SET(sockfd, &setWrite);

        //select
        int isel;
        if (timeout == -1) {
            isel = select(sockfd + 1, 0, &setWrite, 0, 0);
        } else {
            timeval tv;
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            isel = select(sockfd + 1, 0, &setWrite, 0, &tv);
        }
        if (isel > 0 && FD_ISSET(sockfd, &setWrite)) {
            bReturn = true;
        }
    } else {
        bReturn = true;
    }

    if (bReturn) {
        struct sockaddr_in sock_addr;

        // 未知原因导致linux下编译不过，替换为socklen_t
#ifdef WIN32
        int size = sizeof(sockaddr_in);
#else
        socklen_t size = sizeof(sockaddr_in);
#endif
        getsockname(sockfd, (sockaddr*)&sock_addr, &size);

        const char* ip = inet_ntoa(sock_addr.sin_addr);
        localip = ip ? std::string(ip) : std::string("<NULL>");
    }
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
    }

    return bReturn;
}

bool test_connection(const char* local,
                     const char* server, int port, std::string& localip)
{
    SOCKADDR_IN localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    localaddr.sin_family = AF_INET;

    if (local == NULL) {
        localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        localaddr.sin_addr.s_addr = inet_addr(local);
    }
    localaddr.sin_port = htons(0);

    SOCKADDR_IN serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(server);
    serveraddr.sin_port = htons((u_short)port);

    if (serveraddr.sin_addr.s_addr == INADDR_NONE) {
        hostent* lphost = gethostbyname(server);
        if (lphost != NULL) {
            for (int i = 0; lphost->h_addr_list[i]; i++) {
                serveraddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr_list[i])->s_addr;
                if (test_connection(localaddr, serveraddr, 5000, localip)) {
                    return true;
                }
            }
        }
    } else {
        return test_connection(localaddr, serveraddr, 5000, localip);
    }

    return false;
}
#endif //!WIN32