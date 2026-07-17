#pragma once
#include "Includes.h"

namespace Network {
    struct NetworkInfo {
        std::wstring adapterName;
        bool isUp;
        std::string macAddress;
        std::string ipv4;
        std::string ipv6;
        std::string gateway ;
        std::string dns;
        bool dhcpEnabled;

        bool isWifi;
        std::wstring ssid;
        int signalStrength; 
        ULONG linkSpeedMbps;
        std::wstring wifiStandard; 
    };

    inline std::string FormatMAC(BYTE* addr, ULONG len) {
        if (len == 0) return "-1";
        std::stringstream ss;
        for (ULONG i = 0; i < len; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(addr[i]);
            if (i < len - 1) ss << ":";
        }
        std::string mac = ss.str();
        for (auto& c : mac) c = ::toupper(c);
        return mac;
    }

    inline std::wstring TranslateWifiStandard(ULONG phyType) {
        switch (phyType) {
        case 1:  return L"Wi-Fi 1 (802.11b)";
        case 2:  return L"Wi-Fi 2 (802.11a)";
        case 3:  return L"Wi-Fi 3 (802.11g)";
        case 4:  return L"Wi-Fi 4 (802.11n)";
        case 5:  return L"Wi-Fi 5 (802.11ac)";
        case 7:  return L"Wi-Fi 6 (802.11ax)";
        case 8:  return L"Wi-Fi 6E (802.11ax)";
        case 9:  return L"Wi-Fi 7 (802.11be)";
        default: return L"Unknown Standard";
        }
    }

    inline void QueryWifiStatistics(const std::wstring& adapterName, NetworkInfo& info) {
        HANDLE hClient = nullptr;
        DWORD dwMaxClient = 2;
        DWORD dwCurVersion = 0;

        if (WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &hClient) != ERROR_SUCCESS) {
            return;
        }

        PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
        if (WlanEnumInterfaces(hClient, nullptr, &pIfList) == ERROR_SUCCESS) {
            for (DWORD i = 0; i < pIfList->dwNumberOfItems; ++i) {
                WLAN_INTERFACE_INFO ifInfo = pIfList->InterfaceInfo[i];

                if (adapterName.find(ifInfo.strInterfaceDescription) != std::wstring::npos ||
                    std::wstring(ifInfo.strInterfaceDescription).find(adapterName) != std::wstring::npos) {

                    info.isWifi = true;
                    PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = nullptr;
                    DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
                    WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

                    if (WlanQueryInterface(hClient, &ifInfo.InterfaceGuid, wlan_intf_opcode_current_connection,
                        nullptr, &connectInfoSize, (PVOID*)&pConnectInfo, &opCode) == ERROR_SUCCESS) {

                        if (pConnectInfo->isState == wlan_interface_state_connected) {
                            ULONG ssidLen = pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
                            if (ssidLen > 0) {
                                std::string rawSsid(reinterpret_cast<char*>(pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID), ssidLen);
                                info.ssid = std::wstring(rawSsid.begin(), rawSsid.end());
                            }

                            info.signalStrength = static_cast<int>(pConnectInfo->wlanAssociationAttributes.wlanSignalQuality);

                            info.linkSpeedMbps = pConnectInfo->wlanAssociationAttributes.ulTxRate / 1000;

                            info.wifiStandard = TranslateWifiStandard(pConnectInfo->wlanAssociationAttributes.dot11PhyType);
                        }
                        WlanFreeMemory(pConnectInfo);
                    }
                    break;
                }
            }
            WlanFreeMemory(pIfList);
        }
        WlanCloseHandle(hClient, nullptr);
    }

    inline void OpenLocationSettings() {
        ShellExecuteW(
            nullptr,
            L"open",
            L"ms-settings:privacy-location",
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        );
    }

    inline bool ForceTriggerLocationPopup() {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);

        std::wstring registryPath = exePath;
        std::replace(registryPath.begin(), registryPath.end(), L'\\', L'#');

        std::wstring subKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\location\\NonPackaged\\" + registryPath;

        LSTATUS status = RegDeleteKeyW(HKEY_CURRENT_USER, subKey.c_str());

        if (status != ERROR_SUCCESS) {
            std::wstring exeName = std::filesystem::path(exePath).filename().wstring();
            std::wstring fallbackSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\location\\NonPackaged\\" + exeName;
            RegDeleteKeyW(HKEY_CURRENT_USER, fallbackSubKey.c_str());
        }

        HANDLE hClient = nullptr;
        DWORD dwMaxClient = 2;
        DWORD dwCurVersion = 0;
        if (WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &hClient) == ERROR_SUCCESS) {
            PWLAN_INTERFACE_INFO_LIST pIfList = nullptr;
            WlanEnumInterfaces(hClient, nullptr, &pIfList);
            if (pIfList) WlanFreeMemory(pIfList);
            WlanCloseHandle(hClient, nullptr);
        }

        return true;
    }

    inline std::vector<NetworkInfo> GetLocalAdapters() {
        std::vector<NetworkInfo> adapters;
        ULONG outBufLen = 15000;
        std::vector<BYTE> buffer(outBufLen);
        auto* pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

        ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_INCLUDE_GATEWAYS;
        DWORD status = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
        if (status == ERROR_BUFFER_OVERFLOW) {
            buffer.resize(outBufLen);
            pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
            status = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses, &outBufLen);
        }

        if (status == NO_ERROR) {
            for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr; pCurr = pCurr->Next) {
                if (pCurr->OperStatus != IfOperStatusUp || pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
                    continue;

                NetworkInfo info;
                info.adapterName = pCurr->FriendlyName;
                info.isUp = true;

                info.macAddress = FormatMAC(pCurr->PhysicalAddress, pCurr->PhysicalAddressLength);

                info.dhcpEnabled = (pCurr->Dhcpv4Enabled != 0);

                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    sockaddr* sa = pUnicast->Address.lpSockaddr;
                    if (sa->sa_family == AF_INET) {
                        sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(sa);
                        char ipStr[INET_ADDRSTRLEN] = { 0 };
                        inet_ntop(AF_INET, &(sa_in->sin_addr), ipStr, INET_ADDRSTRLEN);
                        info.ipv4 = ipStr;
                    }
                    else if (sa->sa_family == AF_INET6) { 
                        sockaddr_in6* sa_in6 = reinterpret_cast<sockaddr_in6*>(sa);
                        char ipStr[INET6_ADDRSTRLEN] = { 0 };
                        inet_ntop(AF_INET6, &(sa_in6->sin6_addr), ipStr, INET6_ADDRSTRLEN);
                        info.ipv6 = ipStr;
                    }
                }

                if (pCurr->FirstGatewayAddress && pCurr->FirstGatewayAddress->Address.lpSockaddr) {
                    sockaddr* sa = pCurr->FirstGatewayAddress->Address.lpSockaddr;
                    if (sa->sa_family == AF_INET) {
                        sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(sa);
                        char gwStr[INET_ADDRSTRLEN] = { 0 };
                        inet_ntop(AF_INET, &(sa_in->sin_addr), gwStr, INET_ADDRSTRLEN);
                        info.gateway = gwStr;
                    }
                }

                if (pCurr->FirstDnsServerAddress && pCurr->FirstDnsServerAddress->Address.lpSockaddr) {
                    sockaddr* sa = pCurr->FirstDnsServerAddress->Address.lpSockaddr;
                    if (sa->sa_family == AF_INET) {
                        sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(sa);
                        char dnsStr[INET_ADDRSTRLEN] = { 0 };
                        inet_ntop(AF_INET, &(sa_in->sin_addr), dnsStr, INET_ADDRSTRLEN);
                        info.dns = dnsStr;
                    }
                }

                info.linkSpeedMbps = static_cast<ULONG>(pCurr->TransmitLinkSpeed / 1000000ULL);

                if (pCurr->IfType == IF_TYPE_IEEE80211) {
                    QueryWifiStatistics(pCurr->Description, info);
                }

                adapters.push_back(info);
            }
        }
        return adapters;
    }
}