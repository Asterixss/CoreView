#pragma once
#include "Includes.h"

namespace Helpers {

    inline std::wstring GenerateRandomPrefix() {
        static const wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> dist(0, sizeof(alphabet) / sizeof(wchar_t) - 2);

        std::wstring result;
        for (int i = 0; i < 5; ++i) {
            result += alphabet[dist(generator)];
        }
        return result;
    }

    inline bool SaveConsoleToFile(std::wstring& outFileName) {
        wchar_t exePath[MAX_PATH];
        if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) return false;

        std::wstring directoryPath = exePath;
        size_t lastSlash = directoryPath.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            directoryPath = directoryPath.substr(0, lastSlash + 1);
        }
        else {
            directoryPath = L""; 
        }

        std::wstring fileName = GenerateRandomPrefix() + L"CV-CoreView.txt";
        std::wstring fullPath = directoryPath + fileName;
        outFileName = fileName; 

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

        std::wofstream outFile(fullPath);
        if (!outFile.is_open()) return false;

        outFile << consoleText;
        outFile.close();
        return true;
    }
}