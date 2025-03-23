#pragma once

//std
#include <stdexcept>
#include <string>

//fmtlib
#include "fmt/format.h"

namespace lib {
    template<typename... TArgs>
    std::string format(const std::string& fmt, TArgs... args) {
        return fmt::vformat(fmt, fmt::make_format_args(args...));
    }
}

#define RUNTIME_ERROR(fmt) \
std::runtime_error(lib::format(std::string("{}\n{}\n") + fmt, __FILE__, __LINE__))

#define FMT_RUNTIME_ERROR(fmt, ...) \
std::runtime_error(lib::format(std::string("{}\n{}\n") + fmt, __FILE__, __LINE__, __VA_ARGS__))
