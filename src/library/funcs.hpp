#pragma once

//std
#include <charconv>
#include <concepts>
#include <stdexcept>
#include <string>
#include <vector>

//toml11
#include "toml.hpp"

namespace lib::ext {
#include "library/crc64.hpp"
}

namespace lib {
    inline size_t hash_string(const std::string& what) {
        return ext::crc64(0, what.data(), what.size());
    }
    constexpr size_t hash_cstr(const char* what, size_t length) {
        return ext::crc64(0, what, length);
    }

    inline std::vector<std::string_view> split_as_view(const std::string_view& source, char delim) {
        std::vector<std::string_view> result;
        size_t pos = 0;

        for (size_t i = 0; i < source.size(); i++) {
            if (source[i] == delim) {
                result.emplace_back(source.data() + pos, source.data() + i);
                pos = i + 1;
            }
        }
        if (pos != source.size())
            result.emplace_back(source.data() + pos, source.data() + source.size());

        return result;
    }
    inline std::vector<std::string> split_as_copy(const std::string_view& source, char delim) {
        std::vector<std::string> result;
        size_t pos = 0;

        for (size_t i = 0; i < source.size(); i++) {
            if (source[i] == delim) {
                result.emplace_back(source.begin() + pos, source.begin() + i);
                pos = i + 1;
            }
        }
        if (pos != source.size())
            result.emplace_back(source.begin() + pos, source.end());

        return result;
    }

    // TODO: error handling
    template<std::integral TResult>
    TResult sv_to(const std::string_view& str) {
        TResult result;
        auto err = std::from_chars(str.data(), str.data() + str.size(), result);
        return result;
    }

    inline toml::value load_by_id(const std::string& folder, uint64_t id) {
        std::fstream file(folder + '/' + std::to_string(id) + ".toml", std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("file is not found");
        return toml::parse(file);
    }
}
