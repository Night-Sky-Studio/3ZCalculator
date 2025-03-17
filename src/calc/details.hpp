#pragma once

//std
#include <array>
#include <map>

//zzz
#include "zzz/combat.hpp"
#include "zzz/details.hpp"

namespace calc {
    struct enemy_t {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    struct request_t {
        template<typename T>
        struct cell_t {
            size_t id;
            std::shared_ptr<T> ptr;
        };

        cell_t<zzz::AgentDetails> agent;
        cell_t<zzz::WengineDetails> wengine;
        cell_t<zzz::rotation_details> rotation;
        std::array<zzz::Ddp, 6> ddps;
        std::multimap<size_t, cell_t<zzz::DdsDetails>> dds;
    };
}
