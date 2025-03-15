#pragma once

//std
#include <array>

//zzz
#include "zzz/combat.hpp"
#include "zzz/details.hpp"

namespace backend::details {
    struct enemy {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    struct eval_data {
        const zzz::AgentDetails& agent;
        const zzz::WengineDetails& wengine;
        std::array<zzz::Ddp, 6> drive_disks;
        const zzz::rotation_details& rotation;
        const enemy& enemy;
    };
}

namespace backend {
    using enemy_details = details::enemy;
    using eval_data_details = details::eval_data;
}
