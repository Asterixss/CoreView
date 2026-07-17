#pragma once
#include "Includes.h"

namespace Helpers {
    inline bool ShowConfirmationPopup() {
        int response = MessageBoxW(
            nullptr,                                      
            L"We need to reset permissions and restart the app to scan for Wi-Fi.\n\nDo you want to continue?", 
            L"Permission Reset Needed",                               
            MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL                  
        );

        return (response == IDYES);
    }
}