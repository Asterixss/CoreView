#pragma once
#include "Includes.h"

namespace design {
	inline void ScrollToConsoleTop() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) return;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            short windowWidth = csbi.srWindow.Right - csbi.srWindow.Left;
            short windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top;

            SMALL_RECT newWindowRect;
            newWindowRect.Left = 0;
            newWindowRect.Top = 0;
            newWindowRect.Right = windowWidth;
            newWindowRect.Bottom = windowHeight;

            SetConsoleWindowInfo(hConsole, TRUE, &newWindowRect);
        }
    }
}