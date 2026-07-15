#pragma once
#include "Includes.h"

namespace Win {
    struct WindowsInfo {
        std::wstring computerName;
        std::string osArchitecture;
        std::wstring osEdition;
        std::wstring systemLanguage;
        std::wstring keyboardLayout;
        std::wstring upTime;
        DWORD sockets;
        DWORD cores;
        DWORD logicalProcessors;
        bool virtualizationEnabled;
        DWORD l1CacheSizeKB;
        DWORD l2CacheSizeKB;
        DWORD l3CacheSizeKB;
    };

    inline WindowsInfo GetOSInfo() {
        WindowsInfo info;

        WCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(buffer) / sizeof(buffer[0]);
        if (GetComputerNameExW(ComputerNameDnsHostname, buffer, &size)) {
            info.computerName = buffer;
        }

        SYSTEM_INFO sysInfo;
        GetNativeSystemInfo(&sysInfo);
        switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: info.osArchitecture = "x64"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: info.osArchitecture = "ARM64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: info.osArchitecture = "x86"; break;
        default: info.osArchitecture = "Unknown";
        }

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            WCHAR edition[256];
            DWORD bufSize = sizeof(edition);
            if (RegQueryValueExW(hKey, L"ProductName", NULL, NULL, (LPBYTE)edition, &bufSize) == ERROR_SUCCESS) {
                info.osEdition = edition;
            }
            RegCloseKey(hKey);
        }

        WCHAR langBuf[10];
        GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO639LANGNAME, langBuf, 10);
        info.systemLanguage = langBuf;

        // 5. Keyboard Layout
        HKL hkl = GetKeyboardLayout(0);
        WCHAR klName[KL_NAMELENGTH];
        if (GetKeyboardLayoutNameW(klName)) {
            LCID lcid = std::wcstoul(klName, nullptr, 16);
            WCHAR localeName[LOCALE_NAME_MAX_LENGTH];
            if (LCIDToLocaleName(lcid, localeName, LOCALE_NAME_MAX_LENGTH, 0) > 0) {
                WCHAR displayName[256];
                if (GetLocaleInfoEx(localeName, LOCALE_SLOCALIZEDDISPLAYNAME, displayName, 256) > 0) {
                    info.keyboardLayout = displayName;
                }
                else {
                    info.keyboardLayout = klName;
                }
            }
            else {
                info.keyboardLayout = klName;
            }
        }

        ULONGLONG ticks = GetTickCount64();
        ULONGLONG seconds = ticks / 1000;
        ULONGLONG minutes = seconds / 60;
        ULONGLONG hours = minutes / 60;
        ULONGLONG days = hours / 24;
        wchar_t upTimeBuf[128];
        swprintf_s(upTimeBuf, L"%llu days, %02llu:%02llu:%02llu", days, hours % 24, minutes % 60, seconds % 60);
        info.upTime = upTimeBuf;

        info.virtualizationEnabled = IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED) != 0;

        DWORD returnLength = 0;
        GetLogicalProcessorInformation(nullptr, &returnLength);
        if (returnLength > 0) {
            std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> procBuffer(returnLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
            info.sockets = 0;
            info.cores = 0;
            info.logicalProcessors = 0;
            info.l1CacheSizeKB = 0;
            info.l2CacheSizeKB = 0;
            info.l3CacheSizeKB = 0;

            if (GetLogicalProcessorInformation(procBuffer.data(), &returnLength)) {
                for (const auto& item : procBuffer) {
                    if (item.Relationship == RelationProcessorPackage) {
                        info.sockets++;
                    }
                    else if (item.Relationship == RelationProcessorCore) {
                        info.cores++;
                        ULONG_PTR bitmask = item.ProcessorMask;
                        while (bitmask) {
                            if (bitmask & 1) info.logicalProcessors++;
                            bitmask >>= 1;
                        }
                    }
                    else if (item.Relationship == RelationCache) {
                        if (item.Cache.Level == 1) {
                            info.l1CacheSizeKB += item.Cache.Size / 1024;
                        }
                        else if (item.Cache.Level == 2) {
                            info.l2CacheSizeKB += item.Cache.Size / 1024;
                        }
                        else if (item.Cache.Level == 3) {
                            info.l3CacheSizeKB += item.Cache.Size / 1024;
                        }
                    }
                }
            }
            if (info.sockets == 0) info.sockets = 1;
        }

        return info;
    }
}