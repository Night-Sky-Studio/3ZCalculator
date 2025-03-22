#pragma once

//std
#include <string>

//library
#include "library/builder.hpp"
#include "library/cached_memory.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"
#include "zzz/stats_grid.hpp"

namespace zzz::details {
    class Wengine {
        friend class WengineBuilder;

    public:
        uint64_t id() const;
        const std::string& name() const;
        Speciality speciality() const;
        // you can really have access to all stats at once
        const StatsGrid& stats() const;

    protected:
        uint64_t m_id;
        std::string m_name;
        Rarity m_rarity;
        Speciality m_speciality;
        StatsGrid m_stats;
    };

    class WengineBuilder : public lib::IBuilder<Wengine> {
    public:
        WengineBuilder& set_id(uint64_t id);
        WengineBuilder& set_name(std::string name);
        WengineBuilder& set_rarity(Rarity rarity);

        WengineBuilder& set_speciality(Speciality speciality);
        WengineBuilder& set_speciality(std::string_view speciality);

        WengineBuilder& set_main_stat(const StatsGrid& main_stat);
        WengineBuilder& set_sub_stat(const StatsGrid& sub_stat);
        WengineBuilder& set_passive_stats(const StatsGrid& stats);

        bool is_built() const override;
        Wengine&& get_product() override;

    private:
        struct {
            bool id            : 1 = false;
            bool name          : 1 = false;
            bool rarity        : 1 = false;
            bool speciality    : 1 = false;
            bool main_stat     : 1 = false;
            bool sub_stat      : 1 = false;
            bool passive_stats : 1 = false;
        } _is_set;
    };
}

namespace zzz {
    using WengineDetails = details::Wengine;

    class WenginePtr : public lib::MObject {
    public:
        explicit WenginePtr(const std::string& name);

        bool load_from_string(const std::string& input, size_t mode) override;
    };
}
