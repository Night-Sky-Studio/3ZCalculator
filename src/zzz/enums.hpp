#pragma once

//std
#include <concepts>
#include <cstdint>
#include <string>

//magic_enum
#include "magic_enum/magic_enum.hpp"

//lib
#include "library/format.hpp"

namespace zzz {
    class Element {
    public:
        enum class Enum : uint8_t {
            None,
            Phys,
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
        static constexpr Enum _from_string(std::string_view str) {
            return magic_enum::enum_cast<Enum>(str).value_or(None);
        }
        static constexpr std::string_view _to_string(Enum val) {
            return magic_enum::enum_name(val);
        }

        Enum _val;
    };
}

namespace zzz {
    // TODO: make common tag for assists
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
        static constexpr Enum _from_string(std::string_view str) {
            return magic_enum::enum_cast<Enum>(str).value_or(Universal);
        }
        static constexpr std::string_view _to_string(Enum val) {
            return magic_enum::enum_name(val);
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
        static constexpr Enum _from_string(std::string_view str) {
            return magic_enum::enum_cast<Enum>(str).value_or(None);
        }
        static constexpr std::string_view _to_string(Enum val) {
            return magic_enum::enum_name(val);
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
        static constexpr Enum _from_string(std::string_view str) {
            return magic_enum::enum_cast<Enum>(str).value_or(NotSet);
        }
        static constexpr std::string_view _to_string(Enum val) {
            return magic_enum::enum_name(val);
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
            // HpTotal = (HpBase (1 + HpRatio) + HpFlat) * HpRatioCombat + HpFlatCombat
            HpTotal, HpBase, HpRatio, HpFlat, HpRatioCombat, HpFlatCombat,
            // AtkTotal = (AtkBase (1 + AtkRatio) + AtkFlat) * AtkRatioCombat + AtkFlatCombat
            AtkTotal, AtkBase, AtkRatio, AtkFlat, AtkRatioCombat, AtkFlatCombat,
            // DefTotal = (DefBase (1 + DefRatio) + DefFlat) * DefRatioCombat + DefFlatCombat
            DefTotal, DefBase, DefRatio, DefFlat, DefRatioCombat, DefFlatCombat,

            // AmTotal = (AmBase (1 + AmRatio) + AmFlat) * AmRatioCombat + AmFlatCombat
            AmTotal, AmBase, AmRatio, AmFlat, AmRatioCombat, AmFlatCombat,
            // anomaly proficiency, anomaly buildup rate and penetration
            Ap, AbRate, AbPen,

            CritRate, CritDmg,

            // same as PenRatio and PenFlat respectively
            DefPenRatio, DefPenFlat,

            // ImpactTotal = ImpactBase * (1 + ImpactRatio)
            ImpactTotal, ImpactBase, ImpactRatio,
            DazeRatio,

            // ErTotal = ErBase * ErRatio
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
        static constexpr Enum _from_string(std::string_view str) {
            return magic_enum::enum_cast<Enum>(str).value_or(None);
        }
        static constexpr std::string_view _to_string(Enum val) {
            return magic_enum::enum_name(val);
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
