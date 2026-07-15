#pragma once
#include "Includes.h"

namespace design {
    void enableVT() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void printRGB(const std::string& text, float offset = 0.0f) {
        for (size_t i = 0; i < text.length(); ++i) {
            float freq = 0.3f;
            int r = static_cast<int>(std::sin(freq * i + offset) * 127 + 128);
            int g = static_cast<int>(std::sin(freq * i + offset + 2.0f) * 127 + 128);
            int b = static_cast<int>(std::sin(freq * i + offset + 4.0f) * 127 + 128);

            std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text[i];
        }
        std::cout << "\033[0m"; 
    }
}