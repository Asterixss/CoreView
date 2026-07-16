#pragma once
#include "Includes.h"

namespace Helpers {
    int WaitForKey(int targetKey = 0) {
        int ch = 0;
        while (true) {
            ch = _getch();
            if (ch == 0 || ch == 0xE0) {
                _getch(); 
                if (targetKey == 0) return ch; 
                continue;
            }

            if (targetKey == 0) {
                return ch;
            }

            if (ch == targetKey) {
                return ch;
            }
        }
    }
}