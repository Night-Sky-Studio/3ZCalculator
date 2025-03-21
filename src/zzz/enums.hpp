#pragma once

//std
#include <cstdint>
#include <stdexcept>
#include <string>

//frozen
#include "frozen/string.h"

namespace zzz {
    enum class StatId : uint8_t {
        None,
        HpTotal, HpBase, HpRatio, HpFlat,
        AtkTotal, AtkBase, AtkRatio, AtkFlat,
        DefTotal, DefBase, DefRatio, DefFlat,

        CritRate, CritDmg,
        DefPenRatio, DefPenFlat,
        // anomaly proficiency, anomaly mastery, anomaly buildup rate and penetration
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
    constexpr frozen::string stat_id_to_cstr(StatId source);
    std::string_view stat_id_to_string(StatId source);

    constexpr StatId cstr_to_stat_id(const frozen::string& source);
    StatId string_to_stat_id(std::string_view source);

    constexpr frozen::string tag_to_cstr(Tag source);
    std::string_view tag_to_string(Tag source);

    constexpr Tag cstr_to_tag(const frozen::string& source);
    Tag string_to_tag(std::string_view source);

    constexpr frozen::string speciality_to_cstr(Speciality source);
    std::string_view speciality_to_string(Speciality source);

    constexpr Speciality cstr_to_speciality(const frozen::string& source);
    Speciality string_to_speciality(std::string_view source);

    constexpr frozen::string element_to_cstr(Element source);
    std::string_view element_to_string(Element source);

    constexpr Element cstr_to_element(const frozen::string& source);
    Element string_to_element(std::string_view source);
}

inline zzz::StatId operator+(zzz::StatId lhs, zzz::Element rhs) {
#ifdef ADDITIONAL_CHECK_MODE
    if (lhs != zzz::StatId::DmgRatio && lhs != zzz::StatId::ResPen)
        throw std::runtime_error("you only can sum DmgRatio or ResPen with Element");
#endif
    return zzz::StatId((uint8_t) lhs + (uint8_t) rhs);
}
