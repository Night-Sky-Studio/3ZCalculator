#include "zzz/stats/grid.hpp"

//std
#include <ranges>

//zzz
#include "zzz/stats/relative.hpp"
#include "zzz/stats/regular.hpp"

namespace zzz {
    // maker

    StatsGrid StatsGrid::make_from(const utl::Json& json, Tag tag) {
        StatsGrid result;

        for (const auto& stat : json.as_array()) {
            const auto& stat_array = stat.as_array();

            // indicator that this is RelativeStat
            if (stat_array.back().is_string()) {
                result.add(RelativeStat::make_from(stat, tag));
            } else
                result.add(RegularStat::make_from(stat, tag));
        }

        return result;
    }

    // ctor

    StatsGrid::StatsGrid(const StatsGrid& another) noexcept {
        add(another);
    }
    StatsGrid& StatsGrid::operator=(const StatsGrid& another) noexcept {
        m_content.clear();
        add(another);
        return *this;
    }

    StatsGrid::StatsGrid(StatsGrid&& another) noexcept {
        m_content = std::move(another.m_content);
    }
    StatsGrid& StatsGrid::operator=(StatsGrid&& another) noexcept {
        m_content = std::move(another.m_content);
        return *this;
    }

    // getter/setter

    double StatsGrid::get_value(qualifier_t key) const {
        auto it = m_content.find(key.hash());
        return it != m_content.end() ? it->second->value() : 0.0;
    }
    void StatsGrid::set(StatPtr&& value) {
        m_content[value->qualifier().hash()] = std::move(value);
    }

    bool StatsGrid::contains(qualifier_t key) const {
        return m_content.contains(key.hash());
    }

    // indexers

    IStat& StatsGrid::at(qualifier_t key) {
        return *m_content[key.hash()];
    }
    const IStat& StatsGrid::at(qualifier_t key) const {
        return *m_content.at(key.hash());
    }

    // data manipulation

    void StatsGrid::add(const StatPtr& stat) {
        size_t key = stat->qualifier().hash();
        auto it = m_content.find(key);
        if (it != m_content.end())
            it->second = it->second->add_as_copy(stat);
        else
            m_content.emplace(key, stat->copy());
    }
    void StatsGrid::add(const StatsGrid& another) {
        for (const auto& stat : another.m_content | std::views::values)
            add(stat);
    }
}
