#pragma once

//std
#include <array>
#include <concepts>
#include <cstdint>
#include <string>

//frozen
#include "frozen/string.h"
#include "frozen/unordered_map.h"

//lib
#include "library/format.hpp"

namespace zzz {
    class Element {
    public:
        enum class Enum : uint8_t {
            None,
            Physical,
            Fire,
            Ice,
            Electric,
            Ether,
            Count
        };
        using enum Enum;

        // ctor

        constexpr Element() : _val(None) {}
        constexpr Element(Enum val) : _val(val) {}
        template<std::integral T>
        constexpr Element(T val) : _val((Enum) val) {}
        constexpr explicit Element(std::string_view val) : _val(_from_string(val)) {}
        explicit Element(const std::string& val) : _val(_from_string(val)) {}

        // assignment

        constexpr Element& operator=(Enum another) {
            _val = another;
            return *this;
        }
        constexpr Element& operator=(size_t another) {
            _val = (Enum) another;
            return *this;
        }
        constexpr Element& operator=(std::string_view another) {
            _val = _from_string(another);
            return *this;
        }

        // converters

        constexpr operator Enum() const { return _val; }
        constexpr operator size_t() const { return (size_t) _val; }
        constexpr explicit operator std::string_view() const { return _to_string(_val); }
        explicit operator std::string() const { return std::string(_to_string(_val)); }

        // comparison

        constexpr bool operator==(Element another) const { return _val == another._val; }
        constexpr bool operator==(Enum another) const { return _val == another; }

    private:
        static constexpr frozen::unordered_map<frozen::string, Enum, 5> from_string_table = {
            { "phys", Physical }, { "fire", Fire }, { "ice", Ice }, { "electric", Electric }, { "ether", Ether }
        };
        static constexpr Enum _from_string(std::string_view str) {
            auto it = from_string_table.find(str);
            return it != from_string_table.end() ? it->second : None;
        }

        static constexpr std::array<frozen::string, 6> to_string_table = {
            "none", "phys", "fire", "ice", "electric", "ether"
        };
        static constexpr std::string_view _to_string(Enum val) {
            const auto& cstr = (size_t) val < to_string_table.size()
                ? to_string_table[(size_t) val]
                : to_string_table.front();
            return { cstr.data(), cstr.end() };
        }

        Enum _val;
    };
}

namespace zzz {
    class Tag {
    public:
        enum class Enum : uint8_t {
            Universal,
            Anomaly,
            Basic,
            Dash, Counter,
            QuickAssist, FollowupAssist, DefensiveAssist, EvasiveAssist,
            Special, ExSpecial,
            Chain, Ultimate,
            Count
        };
        using enum Enum;

        // ctor

        constexpr Tag() : _val(Universal) {}
        constexpr Tag(Enum val) : _val(val) {}
        template<std::integral T>
        constexpr Tag(T val) : _val((Enum) val) {}
        constexpr explicit Tag(std::string_view val) : _val(_from_string(val)) {}
        explicit Tag(const std::string& val) : _val(_from_string(val)) {}

        // assignment

        constexpr Tag& operator=(Enum another) {
            _val = another;
            return *this;
        }
        constexpr Tag& operator=(size_t another) {
            _val = (Enum) another;
            return *this;
        }
        constexpr Tag& operator=(std::string_view another) {
            _val = _from_string(another);
            return *this;
        }

        // converters

        constexpr operator Enum() const { return _val; }
        constexpr operator size_t() const { return (size_t) _val; }
        constexpr explicit operator std::string_view() const { return _to_string(_val); }
        explicit operator std::string() const { return std::string(_to_string(_val)); }

        // comparison

        constexpr bool operator==(Tag another) const { return _val == another._val; }
        constexpr bool operator==(Enum another) const { return _val == another; }

    private:
        static constexpr frozen::unordered_map<frozen::string, Enum, 13> from_string_table = {
            { "universal", Universal },
            { "anomaly", Anomaly },
            { "basic", Basic },
            { "dash", Dash }, { "counter", Counter },
            { "quick_assist", QuickAssist }, { "followup_assist", FollowupAssist },
            { "defensive_assist", DefensiveAssist }, { "evasive_assist", EvasiveAssist },
            { "special", Special }, { "ex_special", ExSpecial },
            { "chain", Chain }, { "ultimate", Ultimate }
        };
        static constexpr Enum _from_string(std::string_view str) {
            auto it = from_string_table.find(str);
            return it != from_string_table.end() ? it->second : Universal;
        }

        static constexpr std::array<frozen::string, 13> to_string_table = {
            "universal",
            "anomaly",
            "basic",
            "dash", "counter",
            "quick_assist", "followup_assist", "defensive_assist", "evasive_assist",
            "special", "ex_special",
            "chain", "ultimate"
        };
        static constexpr std::string_view _to_string(Enum val) {
            const auto& cstr = (size_t) val < to_string_table.size()
                ? to_string_table[(size_t) val]
                : to_string_table.front();
            return { cstr.data(), cstr.end() };
        }

        Enum _val;
    };
}

namespace zzz {
    class Speciality {
    public:
        enum class Enum : uint8_t {
            None,
            Attack,
            Anomaly,
            Stun,
            Support,
            Defense,
            Count
        };
        using enum Enum;

        // ctor

        constexpr Speciality() : _val(None) {}
        constexpr Speciality(Enum val) : _val(val) {}
        template<std::integral T>
        constexpr Speciality(T val) : _val((Enum) val) {}
        constexpr explicit Speciality(std::string_view val) : _val(_from_string(val)) {}
        explicit Speciality(const std::string& val) : _val(_from_string(val)) {}

        // assignment

        constexpr Speciality& operator=(Enum another) {
            _val = another;
            return *this;
        }
        constexpr Speciality& operator=(size_t another) {
            _val = (Enum) another;
            return *this;
        }
        constexpr Speciality& operator=(std::string_view another) {
            _val = _from_string(another);
            return *this;
        }

        // converters

        constexpr operator Enum() const { return _val; }
        constexpr operator size_t() const { return (size_t) _val; }
        constexpr explicit operator std::string_view() const { return _to_string(_val); }
        explicit operator std::string() const { return std::string(_to_string(_val)); }

        // comparison

        constexpr bool operator==(Speciality another) const { return _val == another._val; }
        constexpr bool operator==(Enum another) const { return _val == another; }

    private:
        static constexpr frozen::unordered_map<frozen::string, Enum, 5> from_string_table = {
            { "attack", Attack },
            { "anomaly", Anomaly },
            { "stun", Stun },
            { "support", Support },
            { "defense", Defense }
        };
        static constexpr Enum _from_string(std::string_view str) {
            auto it = from_string_table.find(str);
            return it != from_string_table.end() ? it->second : None;
        }

        static constexpr std::array<frozen::string, 6> to_string_table = {
            "none", "attack", "anomaly", "stun", "support", "defense"
        };
        static constexpr std::string_view _to_string(Enum val) {
            const auto& cstr = (size_t) val < to_string_table.size()
                ? to_string_table[(size_t) val]
                : to_string_table.front();
            return { cstr.data(), cstr.end() };
        }

        Enum _val;
    };
}

namespace zzz {
    class Rarity {
    public:
        enum class Enum : uint8_t {
            NotSet = 0, B = 2, A = 3, S = 4, Count = 5
        };
        using enum Enum;

        // ctor

        constexpr Rarity() : _val(NotSet) {}
        constexpr Rarity(Enum val) : _val(val) {}
        template<std::integral T>
        constexpr Rarity(T val) :
            _val((Enum) val) {
        }
        constexpr Rarity(char val) : _val(_from_char(val)) {}
        constexpr explicit Rarity(std::string_view val) : _val(_from_string(val)) {}
        explicit Rarity(const std::string& val) : _val(_from_string(val)) {}

        // assignment

        constexpr Rarity& operator=(Enum another) {
            _val = another;
            return *this;
        }
        constexpr Rarity& operator=(size_t another) {
            _val = (Enum) another;
            return *this;
        }
        constexpr Rarity& operator=(char another) {
            _val = _from_char(another);
            return *this;
        }
        constexpr Rarity& operator=(std::string_view another) {
            _val = _from_string(another);
            return *this;
        }

        // converters

        constexpr operator Enum() const { return _val; }
        constexpr operator size_t() const { return (size_t) _val; }
        constexpr explicit operator char() const { return _to_char(_val); }
        constexpr explicit operator std::string_view() const { return _to_string(_val); }
        explicit operator std::string() const { return std::string(_to_string(_val)); }

        // comparison

        constexpr bool operator==(Rarity another) const { return _val == another._val; }
        constexpr bool operator==(Enum another) const { return _val == another; }

    private:
        static constexpr frozen::unordered_map<frozen::string, Enum, 3> from_string_table = {
            { "B", B }, { "A", A }, { "S", S }
        };
        static constexpr Enum _from_string(std::string_view str) {
            auto it = from_string_table.find(str);
            return it != from_string_table.end() ? it->second : NotSet;
        }

        static constexpr std::array<frozen::string, 5> to_string_table = {
            "not_set", "not_set", "B", "A", "S"
        };
        static constexpr std::string_view _to_string(Enum val) {
            const auto& cstr = (size_t) val < to_string_table.size()
                ? to_string_table[(size_t) val]
                : to_string_table.front();
            return { cstr.data(), cstr.end() };
        }

        static constexpr Enum _from_char(char c) {
            switch (c) {
            case 'B':
                return B;
            case 'A':
                return A;
            case 'S':
                return S;
            default:
                return NotSet;
            }
        }
        static constexpr char _to_char(Enum val) {
            switch (val) {
            case B:
                return 'B';
            case A:
                return 'A';
            case S:
                return 'S';
            default:
                return '\0';
            }
        }

        Enum _val;
    };
}

namespace zzz {
    class StatId {
    public:
        enum class Enum : uint8_t {
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
        using enum Enum;

        friend Enum operator+(Enum, Element::Enum);

        // ctor

        constexpr StatId() : _val(None) {}
        constexpr StatId(Enum val) : _val(val) {}
        constexpr StatId(size_t val) : _val((Enum) val) {}
        constexpr explicit StatId(std::string_view val) : _val(_from_string(val)) {}
        explicit StatId(const std::string& val) : _val(_from_string(val)) {}

        // assignment

        constexpr StatId& operator=(Enum another) {
            _val = another;
            return *this;
        }
        constexpr StatId& operator=(size_t another) {
            _val = (Enum) another;
            return *this;
        }
        constexpr StatId& operator=(std::string_view another) {
            _val = _from_string(another);
            return *this;
        }

        // converters

        constexpr operator Enum() const { return _val; }
        constexpr operator size_t() const { return (size_t) _val; }
        constexpr explicit operator std::string_view() const { return _to_string(_val); }
        explicit operator std::string() const { return std::string(_to_string(_val)); }

        // comparison

        constexpr bool operator==(StatId another) const { return _val == another._val; }
        constexpr bool operator==(Enum another) const { return _val == another; }

    private:
        static constexpr frozen::unordered_map<frozen::string, Enum, 44> from_string_table = {
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
        static constexpr Enum _from_string(std::string_view str) {
            auto it = from_string_table.find(str);
            return it != from_string_table.end() ? it->second : None;
        }

        static constexpr std::array<frozen::string, 44> to_string_table = {
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
        static constexpr std::string_view _to_string(Enum val) {
            const auto& cstr = (size_t) val < to_string_table.size()
                ? to_string_table[(size_t) val]
                : to_string_table.front();
            return { cstr.data(), cstr.end() };
        }

        // works only with DmgRatio and ResPen
        static constexpr Enum _add(Enum lhs, Element::Enum rhs) {
            if (lhs != DmgRatio && lhs != ResPen)
                throw RUNTIME_ERROR("you only can sum DmgRatio or ResPen with Element");
            return Enum((size_t) lhs + (size_t) rhs);
        }

        Enum _val;
    };

    inline StatId::Enum operator+(StatId::Enum lhs, Element::Enum rhs) {
        return StatId::_add(lhs, rhs);
    }
}
