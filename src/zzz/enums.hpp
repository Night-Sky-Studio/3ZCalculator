#pragma once

//std
#include <cstdint>
#include <stdexcept>
#include <string>

//frozen
#include "frozen/string.h"

namespace zzz {
    enum class StatType : uint8_t {
        None,
        HpTotal, HpBase, HpRatio, HpFlat,
        AtkTotal, AtkBase, AtkRatio, AtkFlat,
        DefTotal, DefBase, DefRatio, DefFlat,

        CritRate, CritDmg,
        DefPenRatio, DefPenFlat,
        // anomaly proficiency, anomly mastery, anomaly buildup rate and penetration
        Ap, AmTotal, AmBase, AmRatio, AbRate, AbPen,

        ImpactTotal, ImpactBase, ImpactRatio,
        ErTotal, ErBase, ErRatio,
        ShieldEffect, ReceivedDmgReduction, Vulnerability,

        DmgRatio,
        PhysRatio, FireRatio, IceRatio, ElectricRatio, EtherRatio,

        ResPen,
        PhysResPen, FireResPen, IceResPen, ElectricResPen, EtherResPen,

        Count
    };

    enum class Tag : uint8_t {
        Universal,
        Anomaly,
        Basic,
        Dash, Counter,
        QuickAssist, FollowupAssist, DefensiveAssist, EvasiveAssist,
        Special, ExSpecial,
        Chain, Ultimate,
        Count
    };

    enum class Speciality : uint8_t {
        Attack,
        Anomaly,
        Stun,
        Support,
        Defense,
        Count
    };

    enum class Element : uint8_t {
        Physical,
        Fire,
        Ice,
        Electric,
        Ether,
        Count
    };

    enum class Rarity : uint8_t {
        NotSet = 0, B = 2, A = 3, S = 4
    };
}

namespace zzz::convert {
    constexpr frozen::string stat_type_to_cstr(StatType source);
    std::string stat_type_to_string(StatType source);
    constexpr StatType cstr_to_stat_type(const frozen::string& source);
    StatType string_to_stat_type(const std::string& source);

    constexpr frozen::string tag_to_cstr(Tag source);
    std::string tag_to_string(Tag source);
    constexpr Tag cstr_to_tag(const frozen::string& source);
    Tag string_to_tag(const std::string& source);

    constexpr frozen::string speciality_to_cstr(Speciality source);
    std::string speciality_to_string(Speciality source);
    constexpr Speciality cstr_to_speciality(const frozen::string& source);
    Speciality string_to_speciality(const std::string& source);

    constexpr frozen::string element_to_cstr(Element source);
    std::string element_to_string(Element source);
    constexpr Element cstr_to_element(const frozen::string& source);
    Element string_to_element(const std::string& source);

    constexpr char rarity_to_char(Rarity source) {
        switch (source) {
        case Rarity::B:
            return 'B';
        case Rarity::A:
            return 'A';
        case Rarity::S:
            return 'S';
        default:
            return '\0';
        }
    }
    constexpr Rarity char_to_rarity(char source) {
        switch (source) {
        case 'B':
            return Rarity::B;
        case 'A':
            return Rarity::A;
        case 'S':
            return Rarity::S;
        default:
            return Rarity::NotSet;
        }
    }
}

inline zzz::StatType operator+(zzz::StatType lhs, zzz::Element rhs) {
    if (lhs != zzz::StatType::DmgRatio && lhs != zzz::StatType::ResPen)
        throw std::runtime_error("you only can sum DmgRatio or ResPen with Element");
    return zzz::StatType((uint8_t) lhs + (uint8_t) rhs);
}
