#include "zzz/enums.hpp"

//std
#include <array>

//frozen
#include "frozen/unordered_map.h"

namespace zzz::convert_info::inline stat_type_enum {
    using enum StatType;

    static constexpr std::array<frozen::string, (size_t) Count> stat_type2cstr = {
        "none",
        "hp_total", "hp_base", "hp_ratio", "hp_flat",
        "atk_total", "atk_base", "atk_ratio", "atk_flat",
        "def_total", "def_base", "def_ratio", "def_flat",
        "crit_rate", "crit_dmg",
        "def_pen_ratio", "def_pen_flat",
        "ap", "am_total", "am_base", "am_ratio", "ab_rate", "ab_pen",
        "impact_total", "impact_base", "impact_ratio",
        "er_total", "er_base", "er_ratio",
        "shield_effect", "received_dmg_reduction", "vulnerability",
        "dmg_ratio", "phys_ratio", "fire_ratio", "ice_ratio", "electric_ratio", "ether_ratio",
        "res_pen", "phys_res_pen", "fire_res_pen", "ice_res_pen", "electric_res_pen", "ether_res_pen"
    };
    static constexpr frozen::unordered_map<frozen::string, StatType, (size_t) Count> cstr2stat_type = {
        { "none", None },
        { "hp_total", HpTotal }, { "hp_base", HpBase }, { "hp_ratio", HpRatio }, { "hp_flat", HpFlat },
        { "atk_total", AtkTotal }, { "atk_base", AtkBase }, { "atk_ratio", AtkRatio }, { "atk_flat", AtkFlat },
        { "def_total", DefTotal }, { "def_base", DefBase }, { "def_ratio", DefRatio }, { "def_flat", DefFlat },
        { "crit_rate", CritRate }, { "crit_dmg", CritDmg },
        { "def_pen_ratio", DefPenRatio }, { "def_pen_flat", DefPenFlat },
        { "ap", Ap }, { "am_total", AmTotal }, { "am_base", AmBase }, { "am_ratio", AmRatio },
        { "ab_rate", AbRate }, { "ab_pen", AbPen },
        { "impact_total", ImpactTotal }, { "impact_base", ImpactBase }, { "impact_ratio", ImpactRatio },
        { "er_total", ErTotal }, { "er_base", ErBase }, { "er_ratio", ErRatio },
        { "shield_effect", ShieldEffect }, { "received_dmg_reduction", ReceivedDmgReduction },
        { "vulnerability", Vulnerability },
        { "dmg_ratio", DmgRatio }, { "phys_ratio", PhysRatio }, { "fire_ratio", FireRatio },
        { "ice_ratio", IceRatio }, { "electric_ratio", ElectricRatio }, { "ether_ratio", EtherRatio },
        { "res_pen", ResPen }, { "phys_res_pen", PhysResPen }, { "fire_res_pen", FireResPen },
        { "ice_res_pen", IceResPen }, { "electric_res_pen", ElectricResPen }, { "ether_res_pen", EtherResPen }
    };
}

namespace zzz::convert_info::inline tag_enum {
    using enum Tag;

    static constexpr std::array<frozen::string, (size_t) Count> tag2cstr = {
        "universal",
        "anomaly",
        "basic",
        "dash", "counter",
        "quick_assist", "followup_assist", "defensive_assist", "evasive_assist",
        "special", "ex_special",
        "chain", "ultimate"
    };
    static constexpr frozen::unordered_map<frozen::string, Tag, (size_t) Count> cstr2tag = {
        { "universal", Universal },
        { "anomaly", Anomaly },
        { "basic", Basic },
        { "dash", Dash }, { "counter", Counter },
        { "quick_assist", QuickAssist }, { "followup_assist", FollowupAssist },
        { "defensive_assist", DefensiveAssist }, { "evasive_assist", EvasiveAssist },
        { "special", Special }, { "ex_special", ExSpecial },
        { "chain", Chain }, { "ultimate", Ultimate }
    };
}

namespace zzz::convert_info::inline speciality_enum {
    using enum Speciality;

    static constexpr std::array<frozen::string, (size_t) Count> speciality2cstr = {
        "attack", "anomaly", "stun", "support", "defense"
    };
    static constexpr frozen::unordered_map<frozen::string, Speciality, (size_t) Count> cstr2speciality = {
        { "attack", Attack }, { "anomaly", Anomaly }, { "stun", Stun }, { "support", Support }, { "defense", Defense }
    };
}

namespace zzz::convert_info::inline element_enum {
    using enum Element;

    static constexpr std::array<frozen::string, (size_t) Count> element2cstr = {
        "phys", "fire", "ice", "electric", "ether"
    };
    static constexpr frozen::unordered_map<frozen::string, Element, (size_t) Count> cstr2element = {
        { "phys", Physical }, { "fire", Fire }, { "ice", Ice }, { "electric", Electric }, { "ether", Ether }
    };
}

namespace zzz::convert_info::inline rarity_enum {
    using enum Rarity;

    static constexpr frozen::unordered_map<frozen::string, Rarity, (size_t) 3> cstr2rarity = {
        { "B", B }, { "A", A }, { "S", S }
    };
}

namespace zzz::convert {
    // StatType

    constexpr frozen::string stat_type_to_cstr(StatType source) {
        return convert_info::stat_type2cstr[(size_t) source];
    }
    std::string stat_type_to_string(StatType source) {
        const auto& cstr = convert_info::stat_type2cstr[(size_t) source];
        return { cstr.begin(), cstr.end() };
    }

    constexpr StatType cstr_to_stat_type(const frozen::string& source) {
        return convert_info::cstr2stat_type.at(source);
    }
    StatType string_to_stat_type(const std::string& source) {
        return convert_info::cstr2stat_type.at(frozen::string(source.data(), source.size()));
    }

    // Tag

    constexpr frozen::string tag_to_cstr(Tag source) {
        return convert_info::tag2cstr[(size_t) source];
    }
    std::string tag_to_string(Tag source) {
        const auto& cstr = convert_info::tag2cstr[(size_t) source];
        return { cstr.begin(), cstr.end() };
    }

    constexpr Tag cstr_to_tag(const frozen::string& source) {
        return convert_info::cstr2tag.at(source);
    }
    Tag string_to_tag(const std::string& source) {
        return convert_info::cstr2tag.at(frozen::string(source.data(), source.size()));
    }

    // Speciality

    constexpr frozen::string speciality_to_cstr(Speciality source) {
        return convert_info::speciality2cstr[(size_t) source];
    }
    std::string speciality_to_string(Speciality source) {
        const auto& cstr = convert_info::speciality2cstr[(size_t) source];
        return { cstr.begin(), cstr.end() };
    }

    constexpr Speciality cstr_to_speciality(const frozen::string& source) {
        return convert_info::cstr2speciality.at(source);
    }
    Speciality string_to_speciality(const std::string& source) {
        return convert_info::cstr2speciality.at(frozen::string(source.data(), source.size()));
    }

    // Element

    constexpr frozen::string element_to_cstr(Element source) {
        return convert_info::element2cstr[(size_t) source];
    }
    std::string element_to_string(Element source) {
        const auto& cstr = convert_info::element2cstr[(size_t) source];
        return { cstr.begin(), cstr.end() };
    }

    constexpr Element cstr_to_element(const frozen::string& source) {
        return convert_info::cstr2element.at(source);
    }
    Element string_to_element(const std::string& source) {
        return convert_info::cstr2element.at(frozen::string(source.data(), source.size()));
    }

    // Rarity

    std::string rarity_to_string(Rarity source) {
        switch (source) {
        case Rarity::B:
            return "B";
        case Rarity::A:
            return "A";
        case Rarity::S:
            return "S";
        default:
            return "";
        }
    }
    Rarity string_to_rarity(const std::string& source) {
        auto it = convert_info::cstr2rarity.find(frozen::string(source.data(), source.size()));
        return it != convert_info::cstr2rarity.end() ? it->second : Rarity::NotSet;
    }
}
