#pragma once
#include "Includes.h"

namespace Drive {
    struct DriveInfo {
        std::wstring letter;
        std::wstring volumeName;
        std::wstring fileSystem;
        std::string driveType;
        ULONGLONG totalBytes;
        ULONGLONG freeBytes;
    };

    inline std::string ResolveDriveType(UINT type) {
        switch (type) {
        case DRIVE_REMOVABLE: return "Removable";
        case DRIVE_FIXED:     return "Fixed (Local Disk)";
        case DRIVE_REMOTE:    return "Network";
        case DRIVE_CDROM:     return "CD/DVD-ROM";
        case DRIVE_RAMDISK:   return "RAM Disk";
        default:              return "Unknown";
        }
    }

    inline std::vector<DriveInfo> GatherDriveInfo() {
        std::vector<DriveInfo> drives;

        DWORD bufferSize = GetLogicalDriveStringsW(0, nullptr);
        if (bufferSize == 0) return drives;

        std::vector<WCHAR> buffer(bufferSize);
        GetLogicalDriveStringsW(bufferSize, buffer.data());

        WCHAR* driveLetter = buffer.data();
        while (*driveLetter) {
            DriveInfo info;
            info.letter = driveLetter;
            info.driveType = ResolveDriveType(GetDriveTypeW(driveLetter));

            WCHAR volName[MAX_PATH] = { 0 };
            WCHAR fsName[MAX_PATH] = { 0 };
            DWORD serialNumber = 0, maxComponentLen = 0, fileSystemFlags = 0;

            if (GetVolumeInformationW(driveLetter, volName, ARRAYSIZE(volName),
                &serialNumber, &maxComponentLen, &fileSystemFlags,
                fsName, ARRAYSIZE(fsName))) {
                info.volumeName = volName;
                info.fileSystem = fsName;
            }
            else {
                info.volumeName = L"N/A";
                info.fileSystem = L"N/A";
            }

            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            if (GetDiskFreeSpaceExW(driveLetter, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                info.totalBytes = totalNumberOfBytes.QuadPart;
                info.freeBytes = totalNumberOfFreeBytes.QuadPart;
            }
            else {
                info.totalBytes = 0;
                info.freeBytes = 0;
            }

            drives.push_back(info);
            driveLetter += wcslen(driveLetter) + 1;
        }

        return drives;
    }
}