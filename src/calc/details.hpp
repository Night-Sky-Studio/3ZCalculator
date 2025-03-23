#pragma once

//std
#include <array>
#include <map>
#include <list>

//zzz
#include "zzz/details.hpp"

namespace calc {
    struct enemy_t {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    template<typename T>
    struct cell_t {
        uint64_t id;
        std::shared_ptr<T> ptr;

        T* operator->() { return ptr.get(); }
        const T* operator->() const { return ptr.get(); }

        T& operator*() { return *ptr; }
        const T& operator*() const { return *ptr; }
    };

    struct request_t {
        cell_t<zzz::Agent> agent;
        cell_t<zzz::Wengine> wengine;
        cell_t<zzz::Rotation> rotation;

        std::multimap<size_t, zzz::DdsPtr&> dds_by_count;
        std::list<cell_t<zzz::Dds>> dds_list;

        std::array<zzz::Ddp, 6> ddps = {};
    };
}
