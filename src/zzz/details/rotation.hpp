#pragma once

//std
#include <cstdint>
#include <list>
#include <memory>
#include <string>

//library
#include "library/converter.hpp"

namespace zzz::details {
    struct rotation_cell {
        std::string command;
        uint64_t index;
    };
    using rotation = std::list<rotation_cell>;
}

namespace zzz {
    using rotation_details = details::rotation;
    using rotation_details_ptr = std::shared_ptr<details::rotation>;
}
