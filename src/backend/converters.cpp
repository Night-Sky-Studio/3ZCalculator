#include "backend/converters.hpp"

//std
#include <map>
#include <ranges>

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/details.hpp"

namespace backend::inline converter_details {
    using rotations_container = std::unordered_map<std::string, std::vector<std::string>>;

    details::rotation_cell to_rotation_cell(const std::vector<std::string_view>& splitted) {
        size_t index = splitted.size() == 1 ? 0 : lib::sv_to<size_t>(splitted[1]);
        return { std::string(splitted[0]), index };
    }
    details::rotation_cell to_rotation_cell(const std::string_view& text) {
        return to_rotation_cell(lib::split_as_view(text, ' '));
    }

    size_t calc_loops(const std::vector<std::string_view>& splitted) {
        return splitted.size() != 1
            ? lib::sv_to<size_t>(splitted[1].starts_with('x') ? splitted[1].substr(1) : splitted[1])
            : 1;
    }
}

namespace backend {
    // ToRotationConverter

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

    // ToEvalDataConverter

    void ToEvalDataConverter::init() {
        for (auto id : characters_ids) {
            auto agent_toml = lib::load_by_id("agents", id);
            _agents.emplace(id, zzz::global::to_agent.from(agent_toml));

            auto rotation_toml = lib::load_by_id("rotations", id);
            _rotations.emplace(id, global::to_rotation.from(rotation_toml));
        }

        for (auto id : wengines_ids) {
            auto toml = lib::load_by_id("wengines", id);
            _wengines.emplace(id, zzz::global::to_wengine.from(toml));
        }

        for (auto id : dds_ids) {
            auto toml = lib::load_by_id("drive_discs", id);
            _dds.emplace(id, zzz::global::to_dds.from(toml));
        }
    }

    eval_data_details ToEvalDataConverter::from(const toml::value& data) const {
        uint64_t agent_id = data.at("primary_agent_id").as_integer();
        std::string agent_id_as_str = std::to_string(agent_id);

        uint64_t wengine_id = data.at("primary_wengine_id").as_integer();

        size_t current_disk = 0;
        std::array<zzz::Ddp, 6> drive_discs;
        for (const auto& [key, value] : data.at(agent_id_as_str).as_table()) {
            const auto& table = value.as_table();
            zzz::combat::DdpBuilder builder;

            builder.set_slot(std::stoul(key));
            builder.set_disc_id(table.at("set").as_integer());
            builder.set_rarity(zzz::convert::char_to_rarity(table.at("rarity").as_string()[0]));
            builder.set_main_stat(
                zzz::convert::string_to_stat_type(table.at("main").as_string()),
                table.at("level").as_integer()
            );

            for (const auto& it : table.at("subs").as_array()) {
                const auto& array = it.as_array();
                builder.add_sub_stat(
                    zzz::convert::string_to_stat_type(array[0].as_string()),
                    array[1].as_integer()
                );
            }

            drive_discs[current_disk++] = builder.get_product();
        }

        return eval_data_details {
            .agent = _agents.at(agent_id),
            .wengine = _wengines.at(wengine_id),
            .drive_disks = drive_discs,
            .rotation = _rotations.at(agent_id),
            .enemy = enemy
        };
    }

    const uint64_t ToEvalDataConverter::characters_ids[1] = { 1091 };
    const uint64_t ToEvalDataConverter::wengines_ids[1] = { 14109 };
    const uint64_t ToEvalDataConverter::dds_ids[3] = { 31000, 32700, 32800 };

    std::unordered_map<size_t, zzz::AgentDetails> ToEvalDataConverter::_agents;
    std::unordered_map<size_t, zzz::WengineDetails> ToEvalDataConverter::_wengines;
    std::unordered_map<size_t, zzz::DdsDetails> ToEvalDataConverter::_dds;
    std::unordered_map<size_t, rotation_details> ToEvalDataConverter::_rotations;

    const enemy_details ToEvalDataConverter::enemy = {
        .dmg_reduction = 0.2,
        .defense = 953,
        .stun_mult = 1.5,
        .res = { 0.2, 0.2, 0.2, 0.2, 0.2 },
        .is_stunned = false
    };
}
