#pragma once
#define _WIN32_WINNT 0x0A00

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <random>
#include <cmath>

// Networking headers
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

// Windows and Hardware headers
#include <windows.h>
#include <winioctl.h>
#include <cwctype>
#include <intrin.h>
#include <dxgi.h>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <wrl/client.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <conio.h>

// Library linkages
#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")