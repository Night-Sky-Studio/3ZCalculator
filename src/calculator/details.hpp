#pragma once

//std
#include <array>
#include <string>

//zzz
#include "zzz/combat.hpp"
#include "zzz/details.hpp"

namespace calculator::details {
    struct enemy {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    struct eval_data {
        std::string agent_id, wengine_id, rotation_id;
        std::array<zzz::Ddp, 6> drive_discs;
        const enemy& enemy;
    };
}

namespace calculator {
    using enemy_details = details::enemy;
    using eval_data_details = details::eval_data;
}
