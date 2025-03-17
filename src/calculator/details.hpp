#pragma once

//std
#include <array>
#include <map>
#include <string>

//zzz
#include "zzz/combat.hpp"
#include "zzz/details.hpp"

namespace calculator {
    struct enemy {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    struct eval_data_details {
        std::string agent_id, wengine_id, rotation_id;
        std::array<zzz::Ddp, 6> drive_discs;
        const enemy& enemy;
    };

    struct eval_data_composed {
        zzz::AgentDetailsPtr agent;
        zzz::WengineDetailsPtr wengine;
        std::multimap<size_t, zzz::DdsDetailsPtr> dds;
        zzz::rotation_details_ptr rotation;
    };
}
