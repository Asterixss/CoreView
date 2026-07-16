#pragma once
#include "Includes.h"

namespace Helpers {
	inline void setWindowsConsole() {
		SetConsoleOutputCP(CP_UTF8);
	}
}