#pragma once
#include "Includes.h"

namespace Helpers {

    inline bool CopyConsoleToClipboard() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) return false;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return false;

        int totalRows = csbi.dwCursorPosition.Y + 1;
        int rowWidth = csbi.dwSize.X;

        std::wstring consoleText;

        for (int y = 0; y < totalRows; ++y) {
            std::vector<wchar_t> rowBuffer(rowWidth);
            DWORD charsRead = 0;
            COORD readCoord = { 0, (SHORT)y };

            if (ReadConsoleOutputCharacterW(hConsole, rowBuffer.data(), rowWidth, readCoord, &charsRead)) {
                std::wstring line(rowBuffer.data(), charsRead);

                size_t endOfText = line.find_last_not_of(L' ');
                if (endOfText != std::wstring::npos) {
                    line = line.substr(0, endOfText + 1);
                }
                else {
                    line.clear(); 
                }

                consoleText += line;
                if (y < totalRows - 1) {
                    consoleText += L"\r\n"; 
                }
            }
        }

        if (consoleText.empty()) return false;

        if (!OpenClipboard(NULL)) return false;
        EmptyClipboard();

        size_t byteCount = (consoleText.size() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, byteCount);
        if (hMem != NULL) {
            void* pMem = GlobalLock(hMem);
            if (pMem != NULL) {
                memcpy(pMem, consoleText.c_str(), byteCount);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
        }
        CloseClipboard();
        return true;
    }
}