#pragma once

//std
#include <memory>
#include <string>

//library
#include "library/converter.hpp"
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    class Wengine {
        friend class WengineBuilder;
        friend class ToWengineConverter;

    public:
        uint64_t id() const;
        const std::string& name() const;
        Speciality speciality() const;
        const stat& main_stat() const;
        const stat& sub_stat() const;
        const StatsGrid& passive_stats() const;

    protected:
        uint64_t m_id;
        std::string m_name;
        Rarity m_rarity;
        Speciality m_speciality;
        stat m_main_stat, m_sub_stat;
        StatsGrid m_passive_stats;
    };

    class WengineBuilder : public lib::IBuilder<Wengine> {
    public:
        WengineBuilder& set_id(uint64_t id);
        WengineBuilder& set_name(std::string name);
        WengineBuilder& set_rarity(Rarity rarity);
        WengineBuilder& set_speciality(Speciality speciality);
        WengineBuilder& set_main_stat(stat main_stat);
        WengineBuilder& set_sub_stat(stat sub_stat);
        WengineBuilder& add_passive_stat(stat passive_stat);
        WengineBuilder& set_passive_stats(StatsGrid passive_stats);

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

    class ToWengineConverter : public lib::IConverter<Wengine, toml::value> {
    public:
        Wengine from(const toml::value& data) override;
    };
}

namespace zzz::global {
    static details::ToWengineConverter to_wengine;
}

namespace zzz {
    using WengineDetails = details::Wengine;
    using WengineDetailsPtr = std::shared_ptr<details::Wengine>;
}
