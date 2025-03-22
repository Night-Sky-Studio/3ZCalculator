#include "library/logger.hpp"

//std
#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

//frozen
#include "frozen/string.h"

//fmtlib
#include "fmt/chrono.h"

//library
#include "library/format.hpp"

namespace backend::logger_details {
    static constexpr std::array<frozen::string, 5> level2cstr = {
        "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
    };
}

namespace backend::logger_convert {
    std::string level_to_string(crow::LogLevel level) {
        const auto& cstr = logger_details::level2cstr[(size_t) level];
        return { cstr.data(), cstr.data() + cstr.size() };
    }
}

namespace backend {
    void Logger::add_log_stream(std::ostream& stream) {
        _ostreams.emplace_back(&stream);
    }

    void Logger::log(std::string message, crow::LogLevel level) {
        std::lock_guard lock(_mutex);
        auto output = lib::format("({:%F %T})[{:<8}] {}\n",
            std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
            logger_convert::level_to_string(level),
            std::move(message)
        );

        for (const auto& os : _ostreams)
            *os << output << std::flush;
    }
}
