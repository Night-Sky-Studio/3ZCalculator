#pragma once

//std
#include <array>
#include <cstdint>
#include <memory>
#include <string>

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    class DriveDiscSet {
        friend class DdsBuilder;
        friend class ToDdsConverter;

    public:
        uint64_t id() const;
        const std::string& name() const;

        const StatsGrid& p2() const;
        const StatsGrid& p4() const;

    protected:
        uint64_t m_id;
        std::string m_name;
        std::array<StatsGrid, 2> m_set_bonuses;
    };

    class DdsBuilder : public lib::IBuilder<DriveDiscSet> {
    public:
        DdsBuilder& set_id(uint64_t id);
        DdsBuilder& set_name(std::string name);
        DdsBuilder& set_p2(StatsGrid bonus);
        DdsBuilder& set_p4(StatsGrid bonus);

        bool is_built() const override;
        DriveDiscSet&& get_product() override;

    private:
        struct {
            bool id   : 1 = false;
            bool name : 1 = false;
            bool p2   : 1 = false;
            bool p4   : 1 = false;
        } _is_set;
    };

    class ToDdsConverter : protected lib::IConverter<DriveDiscSet, toml::value> {
    public:
        DriveDiscSet from(const toml::value& data) const override;
    };
}

namespace zzz::global {
    static const details::ToDdsConverter to_dds;
}

namespace zzz {
    using DdsDetails = details::DriveDiscSet;
    using DdsDetailsPtr = std::shared_ptr<details::DriveDiscSet>;
}
