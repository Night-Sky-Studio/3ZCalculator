#pragma once

//std
#include <memory>
#include <string>

//library
#include "library/adaptor.hpp"
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    class Wengine {
        friend class WengineBuilder;
        friend class WengineAdaptor;

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

    class WengineAdaptor : public lib::IAdaptor<toml::value, Wengine> {
    public:
        toml::value to_t1(const Wengine& data) override;
        Wengine to_t2(const toml::value& data) override;
    };
}

namespace zzz::global {
    static details::WengineAdaptor wengine_adaptor;
}

namespace zzz {
    using WengineDetails = details::Wengine;
    using WengineDetailsPtr = std::shared_ptr<details::Wengine>;
}
