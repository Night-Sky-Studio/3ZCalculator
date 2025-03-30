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

namespace zzz::details {
    // Rotation

    std::span<uint64_t> Rotation::teammates() { return { m_teammates.begin(), m_teammates.end() }; }
    std::span<const rotation_cell> Rotation::cells() const { return { m_content.begin(), m_content.end() }; }

    const rotation_cell& Rotation::operator[](size_t index) const { return m_content[index]; }
    size_t Rotation::size() const { return m_content.size(); }

    // RotationBuilder

    RotationBuilder& RotationBuilder::add_teammate(uint64_t id) {
        m_product->m_teammates.emplace_back(id);
        return *this;
    }

    RotationBuilder& RotationBuilder::add_cell(rotation_cell cell) {
        _temporary_content.emplace_back(std::move(cell));
        return *this;
    }
    RotationBuilder& RotationBuilder::set_content(std::span<const rotation_cell> content) {
        for (const auto& it : content)
            _temporary_content.emplace_back(it);
        return *this;
    }

    bool RotationBuilder::is_built() const {
        return !_temporary_content.empty();
    }

    Rotation&& RotationBuilder::get_product() {
        if (!is_built())
            throw RUNTIME_ERROR("you have to specify at least one action");

        m_product->m_content.reserve(_temporary_content.size());
        while (!_temporary_content.empty()) {
            auto cell = std::move(_temporary_content.front());
            _temporary_content.pop_front();
            m_product->m_content.emplace_back(std::move(cell));
        }

        return IBuilder::get_product();
    }
}

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

    RotationDetails load_rotation_from_json(const utl::Json& json) {
        const auto& table = json.as_object();
        using raw_rotations = std::unordered_map<std::string, std::vector<std::string>>;

        raw_rotations by_name;
        // result is always lowest priority and it's always single value
        std::multimap<size_t, raw_rotations::iterator, std::greater<>> by_priority;

        for (const auto& [k, v] : table.at("rotations").as_object()) {
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

        details::RotationBuilder builder;

        if (auto it = table.find("teammates"); it != table.end()) {
            for (const auto& id : it->second.as_array())
                builder.add_teammate(id.as_integral());
        }

        std::unordered_map<std::string, RotationDetails> rotations;

        // priorities are in reversed order
        for (const auto& v : by_priority | std::views::values) {
            details::RotationBuilder local_builder;

            for (const auto& it : v->second) {
                auto splitted = lib::split_as_view(it, ' ');

                if (auto jt = rotations.find(std::string(splitted[0])); jt != rotations.end()) {
                    for (size_t i = 0; i < calc_loops(splitted); i++)
                        for (size_t j = 0; j < jt->second.size(); j++)
                            local_builder.add_cell(jt->second[j]);
                } else
                    local_builder.add_cell(to_rotation_cell(splitted));
            }

            rotations.emplace(v->first, local_builder.get_product());
        }

        builder.set_content(rotations.at("final").cells());

        return builder.get_product();
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
            auto details = load_rotation_from_json(json);
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
