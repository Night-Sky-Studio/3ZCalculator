#include "zzz/combat/drive_disc_piece.hpp"

//std
#include <stdexcept>

//frozen
#include "frozen/unordered_map.h"
#include "frozen/unordered_set.h"

namespace zzz::combat::drive_disc_info {
    // mss4 - main stat slot 4
    constexpr frozen::unordered_set mss4_limits = {
        StatType::AtkRatio,
        StatType::HpRatio,
        StatType::DefRatio,
        StatType::Ap,
        StatType::CritRate,
        StatType::CritDmg
    };
    // mss5 - main stat slot 5
    constexpr frozen::unordered_set mss5_limits = {
        StatType::AtkRatio,
        StatType::HpRatio,
        StatType::DefRatio,
        StatType::DefPenRatio,
        StatType::PhysRatio,
        StatType::FireRatio,
        StatType::IceRatio,
        StatType::ElectricRatio,
        StatType::EtherRatio
    };
    // mss6 - main stat slot 6
    constexpr frozen::unordered_set mss6_limits = {
        StatType::AtkRatio,
        StatType::HpRatio,
        StatType::DefRatio,
        StatType::AmRatio,
        StatType::ErRatio,
        StatType::ImpactRatio
    };

    constexpr frozen::unordered_map<StatType, std::array<double, 3>, 18> main_stat_conversion_table = {
        { StatType::HpFlat, { 734, 1468, 2200 } },
        { StatType::AtkFlat, { 104, 212, 316 } },
        { StatType::DefFlat, { 60, 124, 184 } },
        { StatType::AtkRatio, { 0.1, 0.2, 0.3 } },
        { StatType::HpRatio, { 0.1, 0.2, 0.3 } },
        { StatType::DefRatio, { 0.16, 0.32, 0.48 } },
        { StatType::Ap, { 32, 60, 92 } },
        { StatType::CritRate, { 0.8, 0.16, 0.24 } },
        { StatType::CritDmg, { 0.16, 0.32, 0.48 } },
        { StatType::DefPenRatio, { 0.08, 0.16, 0.24 } },
        { StatType::PhysRatio, { 0.1, 0.2, 0.3 } },
        { StatType::FireRatio, { 0.1, 0.2, 0.3 } },
        { StatType::IceRatio, { 0.1, 0.2, 0.3 } },
        { StatType::ElectricRatio, { 0.1, 0.2, 0.3 } },
        { StatType::EtherRatio, { 0.1, 0.2, 0.3 } },
        { StatType::AmRatio, { 0.1, 0.2, 0.3 } },
        { StatType::ErRatio, { 0.2, 0.4, 0.6 } },
        { StatType::ImpactRatio, { 0.06, 0.12, 0.18 } },
    };
    constexpr frozen::unordered_map<StatType, std::array<double, 3>, 10> sub_stat_convertion_table = {
        { StatType::AtkFlat, { 7, 15, 19 } },
        { StatType::AtkRatio, { 0.01, 0.02, 0.03 } },
        { StatType::HpFlat, { 39, 79, 112 } },
        { StatType::HpRatio, { 0.01, 0.02, 0.03 } },
        { StatType::DefFlat, { 5, 10, 15 } },
        { StatType::DefRatio, { 0.016, 0.032, 0.048 } },
        { StatType::CritRate, { 0.008, 0.016, 0.024 } },
        { StatType::CritDmg, { 0.016, 0.032, 0.048 } },
        { StatType::DefPenFlat, { 3, 6, 9 } },
        { StatType::Ap, { 3, 6, 9 } }
    };

    constexpr bool check_ms_limits(uint8_t slot, StatType type) {
        switch (slot) {
        case 1:
            return type == StatType::HpFlat;
        case 2:
            return type == StatType::AtkFlat;
        case 3:
            return type == StatType::DefFlat;
        case 4:
            return mss4_limits.contains(type);
        case 5:
            return mss5_limits.contains(type);
        case 6:
            return mss6_limits.contains(type);
        default:
            return false;
        }
    }
}

namespace zzz::combat {
    // DriveDiscPiece

    uint64_t DriveDiscPiece::disc_id() const { return m_disc_id; }
    uint8_t DriveDiscPiece::slot() const { return m_slot; }
    Rarity DriveDiscPiece::rarity() const { return m_rarity; }
    const stat& DriveDiscPiece::main_stat() const { return m_main_stat; }
    const stat& DriveDiscPiece::sub_stat(size_t index) const { return m_sub_stats[index]; }

    // DriveDiscPieceBuilder

    DdpBuilder& DdpBuilder::set_disc_id(uint64_t disc_id) {
        m_product->m_disc_id = disc_id;
        _is_set.disc_id = true;
        return *this;
    }
    DdpBuilder& DdpBuilder::set_slot(uint8_t slot) {
        m_product->m_slot = slot;
        _is_set.slot = true;
        return *this;
    }
    DdpBuilder& DdpBuilder::set_rarity(Rarity rarity) {
        m_product->m_rarity = rarity;
        _is_set.rarity = true;
        return *this;
    }
    DdpBuilder& DdpBuilder::set_main_stat(StatType type, uint8_t level) {
        if (m_product->m_slot == 0 || m_product->m_rarity == Rarity::NotSet)
            throw std::runtime_error("you have to specify slot and rarity first");
        if (!drive_disc_info::check_ms_limits(m_product->m_slot, type))
            throw std::runtime_error("for this slot main stat doesn't exist");

        m_product->m_main_stat = {
            .value = drive_disc_info::main_stat_conversion_table.at(type)[(size_t) m_product->m_rarity - 2],
            .type = type,
            .tag = Tag::Universal
        };
        _is_set.main_stat = true;
        return *this;
    }
    DdpBuilder& DdpBuilder::add_sub_stat(StatType type, uint8_t level) {
        if (m_product->m_slot == 0 || m_product->m_rarity == Rarity::NotSet)
            throw std::runtime_error("you have to specify slot and main stat first");
        if (m_product->m_main_stat.type == type)
            throw std::runtime_error("you can't have same main and sub stat");

        auto rarity_index = (size_t) m_product->m_rarity - 2;
        m_product->m_sub_stats[_current_sub_stat++] = {
            .value = drive_disc_info::sub_stat_convertion_table.at(type)[rarity_index] * (level + 1),
            .type = type,
            .tag = Tag::Universal
        };
        return *this;
    }

    bool DdpBuilder::is_built() const {
        return _is_set.disc_id
            && _is_set.slot
            && _is_set.rarity
            && _is_set.main_stat
            && _current_sub_stat >= (size_t) m_product->m_rarity - 1;
    }
    DriveDiscPiece&& DdpBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify slot, main stat and at least 3 sub stats");
        return IBuilder::get_product();
    }
}
