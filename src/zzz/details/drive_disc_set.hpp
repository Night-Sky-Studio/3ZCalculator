#pragma once

//std
#include <array>
#include <cstdint>
#include <memory>
#include <string>

//toml11
#include "toml.hpp"

//library
#include "library/adaptor.hpp"
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    class DriveDiscSet {
        friend class DriveDiscSetBuilder;
        friend class DriveDiscSetAdaptor;

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

    class DriveDiscSetBuilder : public lib::IBuilder<DriveDiscSet> {
    public:
        DriveDiscSetBuilder& set_id(uint64_t id);
        DriveDiscSetBuilder& set_name(std::string name);
        DriveDiscSetBuilder& set_p2(StatsGrid bonus);
        DriveDiscSetBuilder& set_p4(StatsGrid bonus);

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

    class DriveDiscSetAdaptor : protected lib::IAdaptor<toml::value, DriveDiscSet> {
    public:
        toml::value to_t1(const DriveDiscSet& data) override;
        DriveDiscSet to_t2(const toml::value& data) override;
    };
}

namespace zzz::global {
    static details::DriveDiscSetAdaptor dds_adaptor;
}

namespace zzz {
    using DdsDetails = details::DriveDiscSet;
    using DdsDetailsPtr = std::shared_ptr<details::DriveDiscSet>;
}
