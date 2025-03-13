#include "zzz/enums.hpp"

//std
#include <array>

//frozen
#include "frozen/unordered_map.h"

namespace zzz::convert_info {
    static constexpr std::array<frozen::string, (size_t) StatType::Count> stat_type2cstr = {
        "none",
        "hp_total",
        "hp_base",
        "hp_ratio",
        "hp_flat",
        "atk_total",
        "atk_base",
        "atk_ratio",
        "atk_flat",
        "def_total",
        "def_base",
        "def_ratio",
        "def_flat",
        "crit_rate",
        "crit_dmg",
        "def_pen_ratio",
        "def_pen_flat",
        "ap",
        "am_total",
        "am_base",
        "am_ratio",
        "ab_rate",
        "ab_pen",
        "impact_total",
        "impact_base",
        "impact_ratio",
        "er_total",
        "er_base",
        "er_ratio",
        "shield_effect",
        "received_dmg_reduction",
        "vulnerability",
        "dmg_ratio",
        "phys_ratio",
        "fire_ratio",
        "ice_ratio",
        "electric_ratio",
        "ether_ratio",
        "res_pen",
        "phys_res_pen",
        "fire_res_pen",
        "ice_res_pen",
        "electric_res_pen",
        "ether_res_pen"
    };
    static constexpr frozen::unordered_map<frozen::string, StatType, (size_t) StatType::Count> cstr2stat_type = {
        { "none", StatType::None },
        { "hp_total", StatType::HpTotal },
        { "hp_base", StatType::HpBase },
        { "hp_ratio", StatType::HpRatio },
        { "hp_flat", StatType::HpFlat },
        { "atk_total", StatType::AtkTotal },
        { "atk_base", StatType::AtkBase },
        { "atk_ratio", StatType::AtkRatio },
        { "atk_flat", StatType::AtkFlat },
        { "def_total", StatType::DefTotal },
        { "def_base", StatType::DefBase },
        { "def_ratio", StatType::DefRatio },
        { "def_flat", StatType::DefFlat },
        { "crit_rate", StatType::CritRate },
        { "crit_dmg", StatType::CritDmg },
        { "def_pen_ratio", StatType::DefPenRatio },
        { "def_pen_flat", StatType::DefPenFlat },
        { "ap", StatType::Ap },
        { "am_total", StatType::AmTotal },
        { "am_base", StatType::AmBase },
        { "am_ratio", StatType::AmRatio },
        { "ab_rate", StatType::AbRate },
        { "ab_pen", StatType::AbPen },
        { "impact_total", StatType::ImpactTotal },
        { "impact_base", StatType::ImpactBase },
        { "impact_ratio", StatType::ImpactRatio },
        { "er_total", StatType::ErTotal },
        { "er_base", StatType::ErBase },
        { "er_ratio", StatType::ErRatio },
        { "shield_effect", StatType::ShieldEffect },
        { "received_dmg_reduction", StatType::ReceivedDmgReduction },
        { "vulnerability", StatType::Vulnerability },
        { "dmg_ratio", StatType::DmgRatio },
        { "phys_ratio", StatType::PhysRatio },
        { "fire_ratio", StatType::FireRatio },
        { "ice_ratio", StatType::IceRatio },
        { "electric_ratio", StatType::ElectricRatio },
        { "ether_ratio", StatType::EtherRatio },
        { "res_pen", StatType::ResPen },
        { "phys_res_pen", StatType::PhysResPen },
        { "fire_res_pen", StatType::FireResPen },
        { "ice_res_pen", StatType::IceResPen },
        { "electric_res_pen", StatType::ElectricResPen },
        { "ether_res_pen", StatType::EtherResPen }
    };

    static constexpr std::array<frozen::string, (size_t) Tag::Count> tag2cstr = {
        "universal",
        "anomaly",
        "basic",
        "dash",
        "counter",
        "quick_assist",
        "assist_followup",
        "defensive_assist",
        "evasive_assist",
        "special",
        "ex_special",
        "chain",
        "ultimate"
    };
    static constexpr frozen::unordered_map<frozen::string, Tag, (size_t) Tag::Count> cstr2tag = {
        { "universal", Tag::Universal },
        { "anomaly", Tag::Anomaly },
        { "basic", Tag::Basic },
        { "dash", Tag::Dash },
        { "counter", Tag::Counter },
        { "quick_assist", Tag::QuickAssist },
        { "followup_assist", Tag::FollowupAssist },
        { "defensive_assist", Tag::DefensiveAssist },
        { "evasive_assist", Tag::EvasiveAssist },
        { "special", Tag::Special },
        { "ex_special", Tag::ExSpecial },
        { "chain", Tag::Chain },
        { "ultimate", Tag::Ultimate }
    };

    static constexpr std::array<frozen::string, (size_t) Speciality::Count> speciality2cstr = {
        "attack",
        "anomaly",
        "stun",
        "support",
        "defense"
    };
    static constexpr frozen::unordered_map<frozen::string, Speciality, (size_t) Speciality::Count> cstr2speciality = {
        { "attack", Speciality::Attack },
        { "anomaly", Speciality::Anomaly },
        { "stun", Speciality::Stun },
        { "support", Speciality::Support },
        { "defense", Speciality::Defense }
    };

    static constexpr std::array<frozen::string, (size_t) Element::Count> element2cstr = {
        "phys",
        "fire",
        "ice",
        "electric",
        "ether"
    };
    static constexpr frozen::unordered_map<frozen::string, Element, (size_t) Element::Count> cstr2element = {
        { "phys", Element::Physical },
        { "fire", Element::Fire },
        { "ice", Element::Ice },
        { "electric", Element::Electric },
        { "ether", Element::Ether }
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
}
