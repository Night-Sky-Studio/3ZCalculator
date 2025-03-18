#include "zzz/details/wengine.hpp"

//std
#include <stdexcept>

//frozen
#include "frozen/unordered_set.h"

namespace zzz::details::wengine_info {
    // ms - main stat
    constexpr frozen::unordered_set ms_limits = { StatType::AtkBase };
}

namespace zzz::details {
    // Wengine

    uint64_t Wengine::id() const { return m_id; }
    const std::string& Wengine::name() const { return m_name; }
    Speciality Wengine::speciality() const { return m_speciality; }
    const stat& Wengine::main_stat() const { return m_main_stat; }
    const stat& Wengine::sub_stat() const { return m_sub_stat; }
    const StatsGrid& Wengine::passive_stats() const { return m_passive_stats; }

    // WengineBuilder

    WengineBuilder& WengineBuilder::set_id(uint64_t id) {
        m_product->m_id = id;
        _is_set.id = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_rarity(Rarity rarity) {
        m_product->m_rarity = rarity;
        _is_set.rarity = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_speciality(Speciality speciality) {
        m_product->m_speciality = speciality;
        _is_set.speciality = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_main_stat(stat main_stat) {
        if (!wengine_info::ms_limits.contains(main_stat.type))
            throw std::runtime_error("this main stat doesn't exist");

        m_product->m_main_stat = main_stat;
        _is_set.main_stat = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_sub_stat(stat sub_stat) {
        m_product->m_sub_stat = sub_stat;
        _is_set.sub_stat = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::add_passive_stat(stat passive_stat) {
        m_product->m_passive_stats.emplace(passive_stat);
        _is_set.passive_stats = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_passive_stats(StatsGrid passive_stats) {
        m_product->m_passive_stats = std::move(passive_stats);
        _is_set.passive_stats = true;
        return *this;
    }

    bool WengineBuilder::is_built() const {
        return _is_set.id
            && _is_set.name
            && _is_set.rarity
            && _is_set.speciality
            && _is_set.main_stat
            && _is_set.sub_stat
            && _is_set.passive_stats;
    }
    Wengine&& WengineBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, speciality and main, sub and passive stats");

        return IBuilder::get_product();
    }

    // WengineAdaptor

    Wengine ToWengineConverter::from(const toml::value& data) const {
        WengineBuilder builder;

        builder.set_id(data.at("id").as_integer());
        builder.set_name(data.at("name").as_string());
        builder.set_rarity((Rarity)data.at("rarity").as_integer());
        builder.set_speciality(convert::string_to_speciality(data.at("speciality").as_string()));

        const auto& stats = data.at("stats").as_table();
        builder.set_main_stat(global::to_stat.from(stats.at("main").as_array()));
        builder.set_sub_stat(global::to_stat.from(stats.at("sub").as_array()));
        builder.set_passive_stats(global::to_stats_grid.from(stats.at("passive")));

        return builder.get_product();
    }
}
