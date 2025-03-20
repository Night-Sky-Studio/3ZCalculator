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

    // TODO: properly organize
    /*struct request_t {
        struct {
            uint64_t id = 0;
            zzz::AgentDetailsPtr ptr = nullptr;
        } agent;
        struct {
            uint64_t id = 0;
            zzz::WengineDetailsPtr ptr = nullptr;
        } wengine;
        struct {
            uint64_t id = 0;
            zzz::rotation_details_ptr ptr = nullptr;
        } rotation;
        std::array<zzz::Ddp, 6> ddps;

        struct {
            std::multimap<size_t, zzz::DdsDetailsPtr*> by_count;
            std::list<std::tuple<uint64_t, zzz::DdsDetailsPtr>> uniques;
        } dds;
    };*/
}
