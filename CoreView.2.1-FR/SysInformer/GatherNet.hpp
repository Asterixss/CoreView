#pragma once
#include "Includes.h"

namespace Network {
    struct NetworkInfo {
        std::wstring adapterName;
        std::string ipAddress;
        bool isUp;
    };

    inline std::vector<NetworkInfo> GetLocalAdapters() {
        std::vector<NetworkInfo> adapters;
        ULONG outBufLen = 15000;
        std::vector<BYTE> buffer(outBufLen);
        auto* pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

        if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST, NULL, pAddresses, &outBufLen) == NO_ERROR) {
            for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr; pCurr = pCurr->Next) {
                if (pCurr->OperStatus != IfOperStatusUp || pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
                    continue;

                NetworkInfo info;
                info.adapterName = pCurr->FriendlyName;
                info.isUp = true;

                if (pCurr->FirstUnicastAddress) {
                    sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(pCurr->FirstUnicastAddress->Address.lpSockaddr);
                    char ipStr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(sa->sin_addr), ipStr, INET_ADDRSTRLEN);
                    info.ipAddress = ipStr;
                }

                adapters.push_back(info);
            }
        }
        return adapters;
    }
}