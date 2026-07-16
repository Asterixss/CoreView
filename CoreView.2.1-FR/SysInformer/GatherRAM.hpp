#pragma once
#include "Includes.h"

namespace RAM {
    struct RAMStick {
        std::wstring manufacturer;
        std::wstring partNumber;
        std::wstring bankLabel;
        ULONGLONG capacityBytes;
        DWORD speedMHz;
        std::string memoryType;
        std::string formFactor;
    };

    struct RAMInfo {
        DWORD memoryLoadPercent;
        ULONGLONG totalPhysicalBytes;
        ULONGLONG availablePhysicalBytes;
        ULONGLONG totalPageFileBytes;
        ULONGLONG availablePageFileBytes;
        ULONGLONG totalVirtualBytes;
        ULONGLONG availableVirtualBytes;

        std::vector<RAMStick> sticks;
    };

    inline std::string ResolveMemoryType(DWORD typeCode) {
        switch (typeCode) {
        case 20: return "DDR";
        case 21: return "DDR2";
        case 24: return "DDR3";
        case 26: return "DDR4";
        case 30: return "DDR5";
        case 31: return "LPDDR4";
        case 32: return "LPDDR5";
        default: return "Unknown (" + std::to_string(typeCode) + ")";
        }
    }

    inline std::string ResolveFormFactor(DWORD formCode) {
        switch (formCode) {
        case 8:  return "DIMM";
        case 12: return "SO-DIMM";
        case 13: return "SRIMM";
        default: return "Unknown (" + std::to_string(formCode) + ")";
        }
    }

    inline RAMInfo GatherRAMInfo() {
        RAMInfo info = { 0 };

        MEMORYSTATUSEX memStatus;
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memStatus)) {
            info.memoryLoadPercent = memStatus.dwMemoryLoad;
            info.totalPhysicalBytes = memStatus.ullTotalPhys;
            info.availablePhysicalBytes = memStatus.ullAvailPhys;
            info.totalPageFileBytes = memStatus.ullTotalPageFile;
            info.availablePageFileBytes = memStatus.ullAvailPageFile;
            info.totalVirtualBytes = memStatus.ullTotalVirtual;
            info.availableVirtualBytes = memStatus.ullAvailVirtual;
        }

        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        bool coInitialized = SUCCEEDED(hr);

        if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
            hr = CoInitializeSecurity(
                NULL, -1, NULL, NULL,
                RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL, EOAC_NONE, NULL
            );

            IWbemLocator* pLocator = NULL;
            hr = CoCreateInstance(
                CLSID_WbemLocator, 0,
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, (LPVOID*)&pLocator
            );

            if (SUCCEEDED(hr)) {
                IWbemServices* pServices = NULL;
                hr = pLocator->ConnectServer(
                    _bstr_t(L"ROOT\\CIMV2"),
                    NULL, NULL, 0, NULL, 0, 0, &pServices
                );

                if (SUCCEEDED(hr)) {
                    hr = CoSetProxyBlanket(
                        pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                        NULL, EOAC_NONE
                    );

                    IEnumWbemClassObject* pEnumerator = NULL;
                    hr = pServices->ExecQuery(
                        _bstr_t("WQL"),
                        _bstr_t("SELECT * FROM Win32_PhysicalMemory"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        NULL, &pEnumerator
                    );

                    if (SUCCEEDED(hr)) {
                        IWbemClassObject* pclsObj = NULL;
                        ULONG uReturn = 0;

                        while (pEnumerator) {
                            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                            if (0 == uReturn) break;

                            RAMStick stick;
                            VARIANT vtProp;

                            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) stick.manufacturer = vtProp.bstrVal;
                            VariantClear(&vtProp);

                        
                            hr = pclsObj->Get(L"PartNumber", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                                stick.partNumber = vtProp.bstrVal;
                                while (!stick.partNumber.empty() && iswspace(stick.partNumber.back())) {
                                    stick.partNumber.pop_back();
                                }
                            }
                            VariantClear(&vtProp);

            
                            hr = pclsObj->Get(L"BankLabel", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) stick.bankLabel = vtProp.bstrVal;
                            VariantClear(&vtProp);

                
                            hr = pclsObj->Get(L"Capacity", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) stick.capacityBytes = std::stoull(vtProp.bstrVal);
                            VariantClear(&vtProp);

                            hr = pclsObj->Get(L"ConfiguredClockSpeed", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_I4) stick.speedMHz = vtProp.lVal;
                            VariantClear(&vtProp);

                        
                            hr = pclsObj->Get(L"SMBIOSMemoryType", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_I4) stick.memoryType = ResolveMemoryType(vtProp.lVal);
                            VariantClear(&vtProp);


                            hr = pclsObj->Get(L"FormFactor", 0, &vtProp, 0, 0);
                            if (SUCCEEDED(hr) && vtProp.vt == VT_I4) stick.formFactor = ResolveFormFactor(vtProp.lVal);
                            VariantClear(&vtProp);

                            info.sticks.push_back(stick);
                            pclsObj->Release();
                        }
                        pEnumerator->Release();
                    }
                    pServices->Release();
                }
                pLocator->Release();
            }
            if (coInitialized) CoUninitialize();
        }

        return info;
    }
}