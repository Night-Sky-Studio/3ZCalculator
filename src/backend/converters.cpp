#include "backend/converters.hpp"

//std
#include <ranges>

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/details.hpp"

namespace backend {
    void ToEvalDataConverter::init() {
        for (auto id : characters_ids) {
            auto agent_toml = lib::load_by_id("agents", id);
            _agents.emplace(id, global::to_agent.from(agent_toml));

            auto rotation_toml = lib::load_by_id("rotations", id);
            _rotations.emplace(id, global::to_rotation.from(rotation_toml));
        }

        for (auto id : wengines_ids) {
            auto toml = lib::load_by_id("wengines", id);
            _wengines.emplace(id, global::to_wengine.from(toml));
        }

        for (auto id : dds_ids) {
            auto toml = lib::load_by_id("drive_discs", id);
            _dds.emplace(id, global::to_dds.from(toml));
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
    std::unordered_map<size_t, zzz::rotation_details> ToEvalDataConverter::_rotations;

    const enemy_details ToEvalDataConverter::enemy = {
        .dmg_reduction = 0.2,
        .defense = 953,
        .stun_mult = 1.5,
        .res = { 0.2, 0.2, 0.2, 0.2, 0.2 },
        .is_stunned = false
    };
}
