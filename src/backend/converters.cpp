#include "backend/converters.hpp"

//std
#include <ranges>

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/details.hpp"

namespace backend {
    eval_data_details ToEvalDataConverter::from(const toml::value& data) const {
        uint64_t agent_id = data.at("primary_agent_id").as_integer();
        std::string agent_id_as_str = std::to_string(agent_id);

        uint64_t wengine_id = data.at("primary_wengine_id").as_integer();
        std::string wengine_id_as_str = std::to_string(wengine_id);

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
            .agent_id = "agents/" + agent_id_as_str,
            .wengine_id = "wengines/" + wengine_id_as_str,
            .rotation_id = "rotations/" + agent_id_as_str,
            .drive_discs = drive_discs,
            .enemy = enemy
        };
    }

    const enemy_details ToEvalDataConverter::enemy = {
        .dmg_reduction = 0.2,
        .defense = 953,
        .stun_mult = 1.5,
        .res = { 0.2, 0.2, 0.2, 0.2, 0.2 },
        .is_stunned = false
    };
}
