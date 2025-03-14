#pragma once

//std
#include <charconv>
#include <concepts>
#include <ranges>
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

    inline std::vector<std::string_view> split_as_view(const std::string_view& source, const std::string& delim) {
        std::vector<std::string_view> result;
        auto splitted_content = std::views::split(source, ' ');

        for (std::ranges::subrange it : splitted_content)
            result.emplace_back(it.data(), it.data() + it.size());

        return result;
    }
    inline std::vector<std::string> split_as_copy(const std::string_view& source, const std::string& delim) {
        std::vector<std::string> result;
        std::ranges::transform(
            std::views::split(source, ' '),
            std::back_inserter(result),
            [](const auto& it) {
                return std::string(it.begin(), it.end());
            }
        );
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
