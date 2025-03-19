#pragma once

//fmtlib
#include "fmt/format.h"

namespace lib {
    template<typename... TArgs>
    std::string format(const std::string& fmt, TArgs... args) {
        return fmt::vformat(fmt, fmt::make_format_args(args...));
    }
}
