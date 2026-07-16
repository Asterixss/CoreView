#pragma once
#include "Includes.h"

namespace GPU {
    struct GPUInfo {
        std::wstring description;
        SIZE_T dedicatedVideoMemory; // Bytes
        UINT vendorId;
        UINT deviceId;
    };

    inline std::vector<GPUInfo> GatherGPUInfo() {
        std::vector<GPUInfo> gpus;
        Microsoft::WRL::ComPtr<IDXGIFactory> pFactory;

        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
            return gpus;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
        for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);

            GPUInfo info;
            info.description = desc.Description;
            info.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            info.vendorId = desc.VendorId;
            info.deviceId = desc.DeviceId;

            gpus.push_back(info);
        }

        return gpus;
    }
}