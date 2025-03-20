#include "zzz/details/rotation.hpp"

//std
#include <map>
#include <ranges>
#include <unordered_map>

//library
#include "library/string_funcs.hpp"

namespace zzz::details {
    using rotations_container = std::unordered_map<std::string, std::vector<std::string>>;

    rotation_cell to_rotation_cell(const std::vector<std::string_view>& splitted) {
        size_t index = splitted.size() == 1 ? 0 : lib::sv_to<size_t>(splitted[1]);
        return { .command = std::string(splitted[0]), .index = index };
    }
    rotation_cell to_rotation_cell(const std::string_view& text) {
        return to_rotation_cell(lib::split_as_view(text, ' '));
    }

    size_t calc_loops(const std::vector<std::string_view>& splitted) {
        return splitted.size() != 1
            ? lib::sv_to<size_t>(splitted[1].starts_with('x') ? splitted[1].substr(1) : splitted[1])
            : 1;
    }
}
