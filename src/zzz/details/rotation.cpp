#include "zzz/details/rotation.hpp"

//std
#include <map>
#include <ranges>
#include <unordered_map>

//library
#include "library/funcs.hpp"

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

    rotation_details ToRotationConverter::from(const toml::value& data) const {
        rotations_container by_name;
        std::multimap<size_t, rotations_container::iterator> by_priority;

        for (const auto& [key, value] : data.at("custom").as_table()) {
            const auto& table = value.as_table();
            const auto& array = table.at("values").as_array();
            size_t priority = table.at("priority").as_integer();

            std::vector<std::string> on_emplace;
            on_emplace.reserve(array.size());

            for (const auto& it : array)
                on_emplace.emplace_back(it.as_string());

            auto [it, flag] = by_name.emplace(key, std::move(on_emplace));
            if (!flag) continue;
            by_priority.emplace(priority, it);
        }

        std::unordered_map<std::string, rotation_details> mini_rotations;

        for (const auto& value : by_priority | std::views::values) {
            rotation_details on_emplace;

            for (const auto& it : value->second) {
                auto splitted = lib::split_as_view(it, ' ');

                if (auto jt = by_name.find(std::string(splitted[0])); jt != by_name.end()) {
                    for (size_t i = 0; i < calc_loops(splitted); i++)
                        for (const auto& line : jt->second)
                            on_emplace.emplace_back(to_rotation_cell(line));
                } else
                    on_emplace.emplace_back(to_rotation_cell(splitted));
            }

            mini_rotations.emplace(value->first, on_emplace);
        }

        rotation_details result;

        for (const auto& value : data.at("rotation").as_array()) {
            auto splitted = lib::split_as_view(value.as_string(), ' ');

            if (auto it = mini_rotations.find(std::string(splitted[0])); it != mini_rotations.end()) {
                for (size_t i = 0; i < splitted.size() * calc_loops(splitted); i++)
                    for (const auto& line : it->second)
                        result.emplace_back(line);
            } else
                result.emplace_back(to_rotation_cell(splitted));
        }

        return result;
    }
}
