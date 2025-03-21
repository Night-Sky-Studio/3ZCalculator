#pragma once

//std
#include <cstdint>
#include <memory>

//library
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::combat {
    class DriveDiscPiece {
        friend class DdpBuilder;

    public:
        uint64_t disc_id() const;
        uint8_t slot() const;
        Rarity rarity() const;
        const stat& main_stat() const;
        const stat& sub_stat(size_t index) const;

    protected:
        uint64_t m_disc_id;
        uint8_t m_slot;
        Rarity m_rarity;
        stat m_main_stat;
        stat m_sub_stats[4];
    };

    // firstly set slot
    class DdpBuilder : public lib::IBuilder<DriveDiscPiece> {
    public:
        DdpBuilder& set_disc_id(uint64_t disc_id);
        DdpBuilder& set_slot(uint8_t slot);
        DdpBuilder& set_rarity(Rarity rarity);
        DdpBuilder& set_main_stat(StatId type, uint8_t level);
        DdpBuilder& add_sub_stat(StatId type, uint8_t level);

        bool is_built() const override;
        DriveDiscPiece&& get_product() override;

    private:
        struct {
            bool disc_id   : 1 = false;
            bool slot      : 1 = false;
            bool rarity    : 1 = false;
            bool main_stat : 1 = false;
        } _is_set;

        uint8_t _current_sub_stat : 3 = 0;
    };
}

namespace zzz {
    using Ddp = combat::DriveDiscPiece;
    using DdpPtr = std::shared_ptr<combat::DriveDiscPiece>;
}
