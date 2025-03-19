#pragma once

//std
#include <fstream>
#include <stdexcept>
#include <string>

//toml11
#include "toml.hpp"

namespace lib {
    inline toml::value load_by_id(const std::string& folder, uint64_t id) {
        std::fstream file(folder + '/' + std::to_string(id) + ".toml", std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("file is not found");
        return toml::parse(file);
    }
}
