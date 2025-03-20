#pragma once

//std
#include <cstdint>
#include <list>
#include <string>

namespace zzz::details {
    struct rotation_cell {
        std::string command;
        uint64_t index;
    };
    using rotation = std::list<rotation_cell>;
}
