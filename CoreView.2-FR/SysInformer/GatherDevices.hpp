#pragma once
#ifndef CONFIGFLAG_DISABLED
#define CONFIGFLAG_DISABLED 0x00000001
#endif

#include "Includes.h"

namespace Peripheral {
    struct ConnectedDevice {
        std::wstring name;
        std::string connectionType;
        std::wstring category;
    };

    inline bool IsSystemOrNoiseDevice(const std::wstring& name) {
        std::wstring lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);

        const std::vector<std::wstring> blocklist = {
            L"avrcp", L"transport", L"enumerator", L"auflistung", L"dienst", L"service",
            L"profile", L"profil", L"rfcomm", L"tdi", L"gatt", L"attribute", L"attribut",
            L"dfu", L"intel", L"realtek", L"mediatek", L"qualcomm", L"integrated",
            L"built-in", L"internal", L"root hub", L"host controller", L"radio", L"adapter",
            L"eingabegerät", L"input device", L"hid-compliant", L"hid-konformes", L"zugriffsprofil",
            L"generisch", L"generic", L"auflister", L"le-enumerator", L"hid-gerät", L"hid device",
            L"bluetooth device", L"bluetooth-gerät", L"microsoft", L"virtual", L"virtuell"
        };

        for (const auto& blocked : blocklist) {
            if (lowerName.find(blocked) != std::wstring::npos) return true;
        }
        return false;
    }

    inline std::wstring GetDriveLetterForDevice(DEVINST devInst) {
        GUID diskInterfaceGuid = { 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } };
        HDEVINFO hDiskInfo = SetupDiGetClassDevsW(&diskInterfaceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (hDiskInfo == INVALID_HANDLE_VALUE) return L"";

        DWORD diskNumber = -1;
        SP_DEVICE_INTERFACE_DATA interfaceData;
        interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDiskInfo, NULL, &diskInterfaceGuid, i, &interfaceData); ++i) {
            SP_DEVINFO_DATA devInfoData;
            devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            DWORD detailSize = 0;
            SetupDiGetDeviceInterfaceDetailW(hDiskInfo, &interfaceData, NULL, 0, &detailSize, &devInfoData);

            if (devInfoData.DevInst != devInst) {
                continue;
            }

            std::vector<BYTE> buffer(detailSize);
            PSP_DEVICE_INTERFACE_DETAIL_DATA_W pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)buffer.data();
            pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

            if (SetupDiGetDeviceInterfaceDetailW(hDiskInfo, &interfaceData, pDetail, detailSize, NULL, NULL)) {
                HANDLE hDevice = CreateFileW(pDetail->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                if (hDevice != INVALID_HANDLE_VALUE) {
                    STORAGE_DEVICE_NUMBER sdn;
                    DWORD bytesReturned = 0;
                    if (DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &bytesReturned, NULL)) {
                        diskNumber = sdn.DeviceNumber;
                    }
                    CloseHandle(hDevice);
                }
            }
            break;
        }
        SetupDiDestroyDeviceInfoList(hDiskInfo);

        if (diskNumber == -1) return L"";

        DWORD logicalDrives = GetLogicalDrives();
        for (int i = 0; i < 26; ++i) {
            if (logicalDrives & (1 << i)) {
                std::wstring drivePath = L"\\\\.\\?:";
                drivePath[4] = L'A' + i;
                HANDLE hDrive = CreateFileW(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
                if (hDrive != INVALID_HANDLE_VALUE) {
                    STORAGE_DEVICE_NUMBER sdn;
                    DWORD bytesReturned = 0;
                    if (DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &bytesReturned, NULL)) {
                        if (sdn.DeviceNumber == diskNumber) {
                            CloseHandle(hDrive);
                            std::wstring letter = L"";
                            letter += (wchar_t)(L'A' + i);
                            return letter + L":";
                        }
                    }
                    CloseHandle(hDrive);
                }
            }
        }
        return L"";
    }

    inline std::vector<ConnectedDevice> GatherConnectedDevices() {
        std::vector<ConnectedDevice> devices;

        HDEVINFO hDevInfo = SetupDiGetClassDevsW(NULL, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
        if (hDevInfo == INVALID_HANDLE_VALUE) return devices;

        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &devInfoData); ++i) {

            ULONG devStatus = 0;
            ULONG problemNumber = 0;
            if (CM_Get_DevNode_Status(&devStatus, &problemNumber, devInfoData.DevInst, 0) != CR_SUCCESS) {
                continue;
            }
            if (!(devStatus & DN_STARTED)) {
                continue;
            }

            DWORD status = 0;
            if (!SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_CONFIGFLAGS, NULL, (PBYTE)&status, sizeof(status), NULL)) {
            }

            if (status & CONFIGFLAG_DISABLED) continue;

            WCHAR classBuf[256] = { 0 };
            if (!SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_CLASS, NULL, (PBYTE)classBuf, sizeof(classBuf), NULL)) {
                continue;
            }

            std::wstring devClass = classBuf;
            if (devClass != L"AudioEndpoint" && devClass != L"Keyboard" &&
                devClass != L"Mouse" && devClass != L"HIDClass" &&
                devClass != L"Bluetooth" && devClass != L"Image" &&
                devClass != L"Camera" && devClass != L"DiskDrive") {
                continue;
            }

            WCHAR nameBuf[512] = { 0 };
            if (!SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, (PBYTE)nameBuf, sizeof(nameBuf), NULL)) {
                if (!SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_DEVICEDESC, NULL, (PBYTE)nameBuf, sizeof(nameBuf), NULL)) {
                    continue;
                }
            }

            std::wstring name = nameBuf;
            if (name.empty() || IsSystemOrNoiseDevice(name)) continue;

            WCHAR enumBuf[256] = { 0 };
            SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_ENUMERATOR_NAME, NULL, (PBYTE)enumBuf, sizeof(enumBuf), NULL);
            std::wstring enumerator = enumBuf;

            WCHAR hwIdBuf[512] = { 0 };
            SetupDiGetDeviceRegistryPropertyW(hDevInfo, &devInfoData, SPDRP_HARDWAREID, NULL, (PBYTE)hwIdBuf, sizeof(hwIdBuf), NULL);
            std::wstring hwId = hwIdBuf;

            std::string connectionType = "Unknown";
            if (enumerator.find(L"BTH") != std::wstring::npos || hwId.find(L"BTH") != std::wstring::npos) {
                connectionType = "Bluetooth";
            }
            else if (enumerator.find(L"USB") != std::wstring::npos ||
                enumerator.find(L"SCSI") != std::wstring::npos ||
                enumerator.find(L"STOR") != std::wstring::npos ||
                hwId.find(L"USB") != std::wstring::npos) {
                connectionType = "Wired (USB / Cable)";
            }
            else {
                continue;
            }

            std::wstring category = devClass;
            if (devClass == L"DiskDrive") {
                category = L"Storage Drive";
                std::wstring driveLetter = GetDriveLetterForDevice(devInfoData.DevInst);
                if (!driveLetter.empty()) {
                    name += L" (" + driveLetter + L")";
                }
            }

            bool exists = false;
            for (const auto& d : devices) {
                if (d.name == name) {
                    exists = true;
                    break;
                }
            }

            if (!exists) {
                ConnectedDevice dev;
                dev.name = name;
                dev.connectionType = connectionType;
                dev.category = category;
                devices.push_back(dev);
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
        return devices;
    }
}