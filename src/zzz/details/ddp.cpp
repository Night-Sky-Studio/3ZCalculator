#include "zzz/details/ddp.hpp"

//std
#include <stdexcept>

//frozen
#include "frozen/unordered_map.h"
#include "frozen/unordered_set.h"

namespace zzz::combat::drive_disc_info {
    // mss4 - main stat slot 4
    constexpr frozen::unordered_set mss4_limits = {
        StatId::AtkRatio,
        StatId::HpRatio,
        StatId::DefRatio,
        StatId::Ap,
        StatId::CritRate,
        StatId::CritDmg
    };
    // mss5 - main stat slot 5
    constexpr frozen::unordered_set mss5_limits = {
        StatId::AtkRatio,
        StatId::HpRatio,
        StatId::DefRatio,
        StatId::DefPenRatio,
        StatId::PhysRatio,
        StatId::FireRatio,
        StatId::IceRatio,
        StatId::ElectricRatio,
        StatId::EtherRatio
    };
    // mss6 - main stat slot 6
    constexpr frozen::unordered_set mss6_limits = {
        StatId::AtkRatio,
        StatId::HpRatio,
        StatId::DefRatio,
        StatId::AmRatio,
        StatId::ErRatio,
        StatId::ImpactRatio
    };

    constexpr frozen::unordered_map<StatId, std::array<double, 3>, 18> main_stat_conversion_table = {
        { StatId::HpFlat, { 734, 1468, 2200 } },
        { StatId::AtkFlat, { 104, 212, 316 } },
        { StatId::DefFlat, { 60, 124, 184 } },
        { StatId::AtkRatio, { 0.1, 0.2, 0.3 } },
        { StatId::HpRatio, { 0.1, 0.2, 0.3 } },
        { StatId::DefRatio, { 0.16, 0.32, 0.48 } },
        { StatId::Ap, { 32, 60, 92 } },
        { StatId::CritRate, { 0.8, 0.16, 0.24 } },
        { StatId::CritDmg, { 0.16, 0.32, 0.48 } },
        { StatId::DefPenRatio, { 0.08, 0.16, 0.24 } },
        { StatId::PhysRatio, { 0.1, 0.2, 0.3 } },
        { StatId::FireRatio, { 0.1, 0.2, 0.3 } },
        { StatId::IceRatio, { 0.1, 0.2, 0.3 } },
        { StatId::ElectricRatio, { 0.1, 0.2, 0.3 } },
        { StatId::EtherRatio, { 0.1, 0.2, 0.3 } },
        { StatId::AmRatio, { 0.1, 0.2, 0.3 } },
        { StatId::ErRatio, { 0.2, 0.4, 0.6 } },
        { StatId::ImpactRatio, { 0.06, 0.12, 0.18 } },
    };
    constexpr frozen::unordered_map<StatId, std::array<double, 3>, 10> sub_stat_convertion_table = {
        { StatId::AtkFlat, { 7, 15, 19 } },
        { StatId::AtkRatio, { 0.01, 0.02, 0.03 } },
        { StatId::HpFlat, { 39, 79, 112 } },
        { StatId::HpRatio, { 0.01, 0.02, 0.03 } },
        { StatId::DefFlat, { 5, 10, 15 } },
        { StatId::DefRatio, { 0.016, 0.032, 0.048 } },
        { StatId::CritRate, { 0.008, 0.016, 0.024 } },
        { StatId::CritDmg, { 0.016, 0.032, 0.048 } },
        { StatId::DefPenFlat, { 3, 6, 9 } },
        { StatId::Ap, { 3, 6, 9 } }
    };

    constexpr bool check_ms_limits(uint8_t slot, StatId type) {
        switch (slot) {
        case 1:
            return type == StatId::HpFlat;
        case 2:
            return type == StatId::AtkFlat;
        case 3:
            return type == StatId::DefFlat;
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
    // Ddp

    uint64_t Ddp::disc_id() const { return m_disc_id; }
    uint8_t Ddp::slot() const { return m_slot; }
    Rarity Ddp::rarity() const { return m_rarity; }
    const StatsGrid& Ddp::stats() const { return m_stats; }

    // DdpBuilder

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
    DdpBuilder& DdpBuilder::set_main_stat(StatId type, uint8_t level) {
        if (m_product->m_slot == 0 || m_product->m_rarity == Rarity::NotSet)
            throw std::runtime_error("you have to specify slot and rarity first");
        if (!drive_disc_info::check_ms_limits(m_product->m_slot, type))
            throw std::runtime_error("for this slot main stat doesn't exist");

        m_product->m_stats.add_regular(
            drive_disc_info::main_stat_conversion_table.at(type)[(size_t) m_product->m_rarity - 2],
            type,
            Tag::Universal
        );
        _main_stat_id = type;
        return *this;
    }
    DdpBuilder& DdpBuilder::add_sub_stat(StatId type, uint8_t level) {
        if (m_product->m_slot == 0 || m_product->m_rarity == Rarity::NotSet)
            throw std::runtime_error("you have to specify slot and main stat first");
        if (_main_stat_id == type)
            throw std::runtime_error("you can't have same main and sub stat");

        size_t rarity_index = (size_t) m_product->m_rarity - 2;
        m_product->m_stats.add_regular(
            drive_disc_info::sub_stat_convertion_table.at(type)[rarity_index] * (level + 1),
            type,
            Tag::Universal
        );
        _current_sub_stat++;
        return *this;
    }

    bool DdpBuilder::is_built() const {
        return _is_set.disc_id
            && _is_set.slot
            && _is_set.rarity
            && _main_stat_id != StatId::None
            && _current_sub_stat >= (size_t) m_product->m_rarity - 1;
    }
    Ddp&& DdpBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify slot, main stat and at least 3 sub stats");
        return IBuilder::get_product();
    }
}
