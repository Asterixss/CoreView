#pragma once
#include "Includes.h"

namespace GPU {
struct APICompatibility {
        std::wstring directX11; 
        std::wstring openGL;  
        std::wstring vulkan; 
    };

    struct GPUInfo {
        std::wstring description;
        SIZE_T dedicatedVideoMemory; 
        UINT vendorId;
        UINT deviceId;
    };

    inline APICompatibility CheckAPIVersions() {
        APICompatibility comp = { L"-1", L"-1", L"-1" };

        // ==========================================
        // 1. DIRECTX 11 (True Feature Level Query)
        // ==========================================
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };
        D3D_FEATURE_LEVEL outputLevel;
        ID3D11Device* pDevice = nullptr;

        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            featureLevels, ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION, &pDevice, &outputLevel, nullptr
        );

        if (SUCCEEDED(hr) && pDevice) {
            if (outputLevel == D3D_FEATURE_LEVEL_11_1) comp.directX11 = L"11.1";
            else if (outputLevel == D3D_FEATURE_LEVEL_11_0) comp.directX11 = L"11.0";
            pDevice->Release();
        }

        // ==========================================
        // 2. OPENGL (True Hardware Context Query)
        // ==========================================
        HWND hwnd = CreateWindowExW(0, L"STATIC", L"OGL_DUMMY", WS_OVERLAPPED, 0, 0, 1, 1, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        if (hwnd) {
            HDC hdc = GetDC(hwnd);
            PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32 };
            int format = ChoosePixelFormat(hdc, &pfd);
            if (format && SetPixelFormat(hdc, format, &pfd)) {
                HGLRC hglrc = wglCreateContext(hdc);
                if (hglrc && wglMakeCurrent(hdc, hglrc)) {
                    const char* verStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                    if (verStr) {
                        std::string rawVer(verStr);
                        size_t spacePos = rawVer.find(' ');
                        std::string cleanVer = (spacePos != std::string::npos) ? rawVer.substr(0, spacePos) : rawVer;
                        comp.openGL = std::wstring(cleanVer.begin(), cleanVer.end());
                    }
                    wglMakeCurrent(nullptr, nullptr);
                    wglDeleteContext(hglrc);
                }
            }
            ReleaseDC(hwnd, hdc);
            DestroyWindow(hwnd);
        }

        // ==========================================
        // 3. VULKAN (Dynamic Driver Instance Query)
        // ==========================================
        HMODULE hVulkan = LoadLibraryW(L"vulkan-1.dll");
        if (hVulkan) {
            typedef int (WINAPI* PFN_vkEnumerateInstanceVersion)(uint32_t* pApiVersion);
            PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion =
                reinterpret_cast<PFN_vkEnumerateInstanceVersion>(GetProcAddress(hVulkan, "vkEnumerateInstanceVersion"));

            uint32_t version = 0;
            if (vkEnumerateInstanceVersion && vkEnumerateInstanceVersion(&version) == 0) {
                uint32_t major = (version >> 22) & 0x7F;
                uint32_t minor = (version >> 12) & 0x3FF;
                uint32_t patch = version & 0xFFF;

                std::wstringstream ss;
                ss << major << L"." << minor << L"." << patch;
                comp.vulkan = ss.str();
            }
            else if (GetProcAddress(hVulkan, "vkCreateInstance")) {
                comp.vulkan = L"1.0.0";
            }
            FreeLibrary(hVulkan);
        }

        return comp;
    }

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