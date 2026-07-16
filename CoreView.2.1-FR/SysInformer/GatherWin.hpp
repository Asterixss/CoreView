#pragma once
#include "Includes.h"

namespace Win {
    struct DisplayInfo {
        std::wstring deviceName;
        std::wstring deviceString;
        DWORD refreshRate;
        int width;
        int height;
    };

    struct WindowsInfo {
        std::wstring computerName;
        std::string osArchitecture;
        std::wstring osEdition;
        std::wstring systemLanguage;
        std::wstring currentKeyboardLayout;
        std::vector<std::wstring> userAccounts;
        std::vector<std::wstring> availableKeyboardLayouts;
        std::wstring upTime;
        DWORD sockets;
        DWORD cores;
        DWORD logicalProcessors;
        bool virtualizationEnabled;
        DWORD l1CacheSizeKB;
        DWORD l2CacheSizeKB;
        DWORD l3CacheSizeKB;

        int batteryPercentage;

        std::vector<DisplayInfo> displays;
    };

    inline int GetBatteryPercentage() {
        SYSTEM_POWER_STATUS status;
        if (GetSystemPowerStatus(&status)) {
            if (status.BatteryLifePercent != 255) {
                return static_cast<int>(status.BatteryLifePercent);
            }
        }
        return -1;
    }

    inline std::vector<DisplayInfo> GetDisplayInfo() {
        std::vector<DisplayInfo> displays;
        DISPLAY_DEVICEW adapter;
        adapter.cb = sizeof(DISPLAY_DEVICEW);

        for (DWORD adapterIndex = 0; EnumDisplayDevicesW(nullptr, adapterIndex, &adapter, 0); ++adapterIndex) {

            if (!(adapter.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

            DISPLAY_DEVICEW monitor;
            monitor.cb = sizeof(DISPLAY_DEVICEW);

            for (DWORD monitorIndex = 0; EnumDisplayDevicesW(adapter.DeviceName, monitorIndex, &monitor, DISPLAY_DEVICE_ACTIVE); ++monitorIndex) {

                if (monitor.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;

                DisplayInfo info;
                info.deviceName = monitor.DeviceName;
                info.deviceString = monitor.DeviceString;

                DEVMODEW dm;
                ZeroMemory(&dm, sizeof(dm));
                dm.dmSize = sizeof(dm);

                if (EnumDisplaySettingsW(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
                    info.refreshRate = dm.dmDisplayFrequency;
                    info.width = static_cast<int>(dm.dmPelsWidth);
                    info.height = static_cast<int>(dm.dmPelsHeight);
                }
                else {
                    info.refreshRate = 0;
                    info.width = 0;
                    info.height = 0;
                }

                displays.push_back(info);
            }
        }
        return displays;
    }

    inline std::vector<std::wstring> GetLocalUserAccounts() {
        std::vector<std::wstring> users;
        LPUSER_INFO_0 pBuf = nullptr;
        LPUSER_INFO_0 pTmpBuf;
        DWORD dwEntriesRead = 0;
        DWORD dwTotalEntries = 0;
        DWORD dwResumeHandle = 0;
        NET_API_STATUS nStatus;

        do {
            nStatus = NetUserEnum(nullptr, 0, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&pBuf,
                MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

            if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
                pTmpBuf = pBuf;
                for (DWORD i = 0; i < dwEntriesRead; i++) {
                    if (pTmpBuf != nullptr) {
                        users.push_back(pTmpBuf->usri0_name);
                        pTmpBuf++;
                    }
                }
            }
            if (pBuf != nullptr) {
                NetApiBufferFree(pBuf);
                pBuf = nullptr;
            }
        } while (nStatus == ERROR_MORE_DATA);

        return users;
    }

    inline std::wstring GetDisplayNameFromHKL(HKL hkl) {
        LCID lcid = (LCID)((UINT_PTR)hkl & 0xFFFF);
        WCHAR displayName[256];
        if (GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, displayName, 256) > 0) {
            return std::wstring(displayName);
        }
        return L"Unknown Layout";
    }

    inline WindowsInfo GetOSInfo() {
        WindowsInfo info;

        WCHAR buffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(buffer) / sizeof(buffer[0]);
        if (GetComputerNameExW(ComputerNameDnsHostname, buffer, &size)) {
            info.computerName = buffer;
        }

        info.userAccounts = GetLocalUserAccounts();
        info.displays = GetDisplayInfo();
        info.batteryPercentage = GetBatteryPercentage();

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

        info.currentKeyboardLayout = GetDisplayNameFromHKL(GetKeyboardLayout(0));

        UINT nLayouts = GetKeyboardLayoutList(0, NULL);
        if (nLayouts > 0) {
            std::vector<HKL> hklList(nLayouts);
            GetKeyboardLayoutList(nLayouts, hklList.data());
            for (HKL hkl : hklList) {
                info.availableKeyboardLayouts.push_back(GetDisplayNameFromHKL(hkl));
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