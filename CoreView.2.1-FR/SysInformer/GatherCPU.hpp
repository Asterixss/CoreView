#pragma once
#include "Includes.h"

namespace CPU {
    struct CPUInfo {
        std::string brand;
        std::string vendor;
        std::string arch;
        DWORD physicalCores;
        DWORD logicalProcessors;
        DWORD maxClockSpeedMHz;
        bool isHyperThreading;
    };

    inline CPUInfo* GatherCPUInfo() {
        static CPUInfo info;
        int cpuInfo[4];

        char vendor[13] = { 0 };
        __cpuid(cpuInfo, 0);
        memcpy(vendor, &cpuInfo[1], 4);
        memcpy(vendor + 4, &cpuInfo[3], 4);
        memcpy(vendor + 8, &cpuInfo[2], 4);
        info.vendor = vendor;

        char brand[49] = { 0 };
        for (int i = 0; i < 3; ++i) {
            __cpuid(cpuInfo, 0x80000002 + i);
            memcpy(brand + (i * 16), cpuInfo, 16);
        }
        info.brand = brand;

        DWORD length = 0;
        GetLogicalProcessorInformationEx(RelationProcessorPackage, nullptr, &length);
        std::vector<BYTE> buffer(length);
        if (GetLogicalProcessorInformationEx(RelationProcessorPackage, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data(), &length)) {
            auto* infoEx = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer.data();
            info.physicalCores = infoEx->Processor.GroupCount; 
        }

        SYSTEM_INFO sysInfo;
        GetNativeSystemInfo(&sysInfo);
        info.logicalProcessors = sysInfo.dwNumberOfProcessors;
        info.isHyperThreading = (info.logicalProcessors > info.physicalCores);

        switch (sysInfo.wProcessorArchitecture) {
        case 9:  info.arch = "x64"; break;
        case 0:  info.arch = "x86"; break;
        case 12: info.arch = "ARM64"; break;
        default: info.arch = "Unknown";
        }

        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD speed = 0, size = sizeof(DWORD);
            if (RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE)&speed, &size) == ERROR_SUCCESS) {
                info.maxClockSpeedMHz = speed;
            }
            RegCloseKey(hKey);
        }

        return &info;
    }
}