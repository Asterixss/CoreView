#pragma once
#include "Includes.h"

struct UserTime {
    std::string date;
    std::string time;
    std::string timezone;
};

namespace Helpers {

    UserTime GetCurrentTimeInfo() {
        auto now = std::chrono::system_clock::now();
        auto local_zone = std::chrono::current_zone();

        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm{};
        localtime_s(&local_tm, &now_c);

        auto info = local_zone->get_info(now);
        auto offset_hours = std::chrono::duration_cast<std::chrono::hours>(info.offset).count();

        return UserTime{
            std::format("{:02}/{:02}/{:04}", local_tm.tm_mday, local_tm.tm_mon + 1, local_tm.tm_year + 1900),
            std::format("{:02}:{:02}:{:02}", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec),
            std::format("{} (GMT{:+d})", info.abbrev, offset_hours)
        };
    }
}