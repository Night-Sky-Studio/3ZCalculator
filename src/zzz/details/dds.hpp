#pragma once

//std
#include <array>
#include <cstdint>
#include <string>

//library
#include "library/builder.hpp"
#include "library/cached_memory.hpp"

//zzz
#include "zzz/stats/grid.hpp"

namespace zzz::details {
    class Dds {
        friend class DdsBuilder;

    public:
        uint64_t id() const;
        const std::string& name() const;

        const StatsGrid& pc2() const;
        const StatsGrid& pc4() const;

    protected:
        uint64_t m_id;
        std::string m_name;
        std::array<StatsGrid, 2> m_set_bonuses;
    };

    class DdsBuilder : public lib::IBuilder<Dds> {
    public:
        DdsBuilder& set_id(uint64_t id);
        DdsBuilder& set_name(std::string name);
        DdsBuilder& set_pc2(StatsGrid bonus);
        DdsBuilder& set_pc4(StatsGrid bonus);

        bool is_built() const override;
        Dds&& get_product() override;

    private:
        struct {
            bool id   : 1 = false;
            bool name : 1 = false;
            bool p2   : 1 = false;
            bool p4   : 1 = false;
        } _is_set;
    };
}

namespace zzz {
    using DdsDetails = details::Dds;

    class Dds : public lib::MObject {
    public:
        explicit Dds(const std::string& fullname);

        DdsDetails& details();
        const DdsDetails& details() const;

        bool load_from_string(const std::string& input, size_t mode) override;
    };
    using DdsPtr = std::shared_ptr<Dds>;
}
