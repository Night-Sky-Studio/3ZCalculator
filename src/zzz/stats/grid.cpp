#include "zzz/stats/grid.hpp"

//library
#include "library/format.hpp"

//zzz
#include <ranges>

#include "zzz/stats/regular.hpp"

namespace zzz {
    // maker

    StatsGrid StatsGrid::make_from(const utl::json::Array& array) {
        StatsGrid result;

        for (const auto& stat : array) {
            const auto& stat_array = stat.as_array();

            // indicator that this is RelativeStat
            if (stat_array.back().is_string()) {
                throw RUNTIME_ERROR("TODO");
            } else
                result.add(RegularStat::make_from(stat_array));
        }
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

    double StatsGrid::get_value(qualifier_t qualifier) const {
        auto it = m_content.find(qualifier.hash());
        return it != m_content.end() ? it->second->value() : 0.0;
    }
    void StatsGrid::set(StatPtr&& value) {
        m_content[value->qualifier().hash()] = std::move(value);
    }

    // indexers

    IStat& StatsGrid::at(qualifier_t qualifier) {
        return *m_content[qualifier.hash()];
    }
    const IStat& StatsGrid::at(qualifier_t qualifier) const {
        return *m_content.at(qualifier.hash());
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
