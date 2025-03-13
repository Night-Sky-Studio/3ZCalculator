#pragma once

//std
#include <cstdint>

extern "C" {
    struct rotation_packed_data {
        uint64_t size;
        const char** actions;
    };

    struct disc_packed_data {
        uint8_t main_stat_id;
        uint8_t level;
        uint8_t sub_stat_id[4];
        uint8_t sub_stat_level[4];
    };

    struct character_packed_data {
        uint64_t character_id, weapon_id;
        uint64_t set_bonus_id[3];
        disc_packed_data discs[6];
    };

    // if size is 0 and actions is nullptr, uses standard rotation instead
    struct request_packed_data {
        character_packed_data character;
        rotation_packed_data rotation;
    };
}
