#include "backend/logger.hpp"

//std
#include <array>
#include <chrono>
#include <stdexcept>

//frozen
#include "frozen/string.h"

//fmtlib
#include "fmt/chrono.h"

//library
#include "library/funcs.hpp"

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
    Logger::Logger(const std::string& path) :
        _file(path, std::ios::out | std::ios::trunc) {
        if (!_file.is_open())
            throw std::runtime_error("log file is not found");
    }

    void Logger::log(std::string message, crow::LogLevel level) {
        std::lock_guard lock(_mutex);
        _file << lib::format("({:%F %T})[{:<8}] {}\n",
            std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
            logger_convert::level_to_string(level),
            std::move(message)
        ) << std::flush;
    }
}
