#pragma once

//std
#include <cstdint>
#include <list>
#include <string>

//library
#include "library/cached_memory.hpp"

namespace zzz::details {
    struct rotation_cell {
        std::string command;
        uint64_t index;
    };
    using Rotation = std::list<rotation_cell>;
}

namespace zzz {
    using RotationDetails = details::Rotation;

    class Rotation : public lib::MObject {
    public:
        explicit Rotation(const std::string& name);

        bool load_from_string(const std::string& input, size_t mode) override;
    };
    using RotationPtr = std::shared_ptr<Rotation>;
}
