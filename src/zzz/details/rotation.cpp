#include "zzz/details/rotation.hpp"

//std
#include <map>
#include <ranges>
#include <unordered_map>

//utl
#include "utl/json.hpp"

//crow
#include "crow/logging.h"

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

namespace zzz {
    // Service

    details::rotation_cell to_rotation_cell(const std::vector<std::string_view>& splitted) {
        size_t index = splitted.size() == 1 ? 0 : lib::sv_to<size_t>(splitted[1]);
        return { .command = std::string(splitted[0]), .index = index };
    }
    details::rotation_cell to_rotation_cell(const std::string_view& text) {
        return to_rotation_cell(lib::split_as_view(text, ' '));
    }

    size_t calc_loops(const std::vector<std::string_view>& splitted) {
        return splitted.size() != 1
            ? lib::sv_to<size_t>(splitted[1].starts_with('x') ? splitted[1].substr(1) : splitted[1])
            : 1;
    }

    RotationDetails load_from_json(const utl::Json& json) {
        using raw_rotations = std::unordered_map<std::string, std::vector<std::string>>;

        raw_rotations by_name;
        // result is always lowest priority and it's always single value
        std::multimap<size_t, raw_rotations::iterator, std::greater<>> by_priority;

        for (const auto& [k, v] : json.as_object()) {
            const auto& table = v.as_object();
            const auto& array = table.at("vals").as_array();
            size_t priority = table.at("priority").as_integral();

            std::vector<std::string> on_emplace;
            on_emplace.reserve(array.size());

            for (const auto& it : array)
                on_emplace.emplace_back(it.as_string());
            auto [it, flag] = by_name.emplace(k, std::move(on_emplace));
            if (!flag)
                continue;
            by_priority.emplace(priority, it);
        }

        std::unordered_map<std::string, RotationDetails> rotations;

        // priorities are in reversed order
        for (const auto& v : by_priority | std::views::values) {
            RotationDetails on_emplace;

            for (const auto& it : v->second) {
                auto splitted = lib::split_as_view(it, ' ');

                if (auto jt = rotations.find(std::string(splitted[0])); jt != rotations.end()) {
                    for (size_t i = 0; i < calc_loops(splitted); i++)
                        for (const auto& cell : jt->second)
                            on_emplace.emplace_back(cell);
                } else
                    on_emplace.emplace_back(to_rotation_cell(splitted));
            }

            rotations.emplace(v->first, std::move(on_emplace));
        }

        return std::move(rotations.at("final"));
    }

    // Rotation

    Rotation::Rotation(const std::string& name) :
        MObject(lib::format("rotations/{}", name)) {
    }

    RotationDetails& Rotation::details() { return as<RotationDetails>(); }
    const RotationDetails& Rotation::details() const { return as<RotationDetails>(); }

    bool Rotation::load_from_string(const std::string& input, size_t mode) {
        if (mode == 1) {
            auto json = utl::json::from_string(input);
            auto details = load_from_json(json);
            set(std::move(details));
        } else {
#ifdef DEBUG_STATUS
            CROW_LOG_ERROR << lib::format("extension_id {} isn't defined", mode);
#endif
            return false;
        }

        return true;
    }
}
