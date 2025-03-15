#pragma once

//std
#include <cstdint>
#include <list>
#include <memory>
#include <string>

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"

namespace zzz::details {
    struct rotation_cell {
        std::string command;
        uint64_t index;
    };
    using rotation = std::list<rotation_cell>;

    class ToRotationConverter : lib::IConverter<rotation, toml::value> {
    public:
        rotation from(const toml::value& data) const override;
    };
}

namespace zzz {
    using rotation_details = details::rotation;
    using rotation_details_ptr = std::shared_ptr<details::rotation>;
}

namespace global {
    static const zzz::details::ToRotationConverter to_rotation;
}
