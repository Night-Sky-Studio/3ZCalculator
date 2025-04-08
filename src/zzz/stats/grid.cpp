#include "zzz/stats/grid.hpp"

//std
#include <functional>
#include <ranges>

//frozen
#include "frozen/string.h"
#include "frozen/unordered_map.h"

//zzz
#include "zzz/stats/regular.hpp"
#include "zzz/stats/relative.hpp"

using namespace frozen::string_literals;
namespace ph = std::placeholders;

namespace zzz::details {
    static constexpr frozen::unordered_map<StatId::Enum, frozen::string, 2> formulas = {
        { StatId::AtkTotal, "f:(AtkBase * (1 + AtkRatio) + AtkFlat) * (1 + AtkRatioCombat) + AtkFlatCombat"_s },
        { StatId::AmTotal, "f:(AmBase * (1 + AmRatio) + AmFlat) * (1 + AmRatioCombat) + AmFlatCombat"_s }
    };

    StatPtr make_relative(const utl::Json& json, Tag tag, const StatsGrid& lookup_table) {
        auto ptr = RelativeStat::make_from(json, tag);

        // after making relative stat we have to specify lookup table
        auto relative = dynamic_cast<RelativeStat&>(*ptr);
        relative.lookup_table(&lookup_table);

        return ptr;
    }
    StatPtr make_regular(const utl::Json& json, Tag tag) {
        return RegularStat::make_from(json, tag);
    }
}

namespace zzz {
    // maker

    StatPtr StatsGrid::make_defined_relative_stat(StatId id, Tag tag) {
        const auto& formula = details::formulas.at(id);
        return RelativeStat::make(id, tag, 0.0, { formula.data(), formula.size() });
    }

    StatsGrid StatsGrid::make_from(const utl::Json& json, Tag tag) {
        StatsGrid result;

        for (const auto& stat : json.as_array()) {
            const auto& stat_array = stat.as_array();
            result.set(stat_array.back().is_string()
                ? details::make_relative(stat_array, tag, result)
                : details::make_regular(stat_array, tag));
        }

        return result;
    }
    StatsGrid StatsGrid::make_from(const utl::Json& json, std::span<Tag> tags) {
        StatsGrid result;

        for (const auto& stat : json.as_array()) {
            const auto& stat_array = stat.as_array();
            // indicator that this is RelativeStat
            std::function<StatPtr(const utl::Json&, Tag)> maker;
            if (stat_array.back().is_string())
                maker = [&](const utl::Json& json, Tag tag) { return details::make_relative(json, tag, result); };
            else
                maker = details::make_regular;

            for (const auto& tag : tags)
                result.set(maker(stat, tag));
        }

        return result;
    }

    // ctor

    StatsGrid::StatsGrid(const StatsGrid& another) noexcept {
        _copy_from(another);
    }
    StatsGrid& StatsGrid::operator=(const StatsGrid& another) noexcept {
        m_content.clear();
        _copy_from(another);
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
        _set_lookup_table_if_relative(value);
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
        StatPtr* ptr;

        if (it != m_content.end()) {
            it->second = it->second->add_as_copy(stat);
            ptr = &it->second;
        } else {
            auto [jt, flag] = m_content.emplace(key, stat->copy());
            ptr = &jt->second;
        }

        _set_lookup_table_if_relative(*ptr);
    }
    void StatsGrid::add(const StatsGrid& another) {
        for (const auto& stat : another.m_content | std::views::values)
            add(stat);
    }

    void StatsGrid::_copy_from(const StatsGrid& another) {
        m_content.reserve(another.m_content.size());
        for (size_t i = 0; i < another.m_content.size(); i++) {
            const auto& [k, v] = *another.m_content.nth(i);
            m_content.emplace_hint(m_content.begin() + i, k, v->copy());
        }

        for (const auto& [k, v] : another.m_content)
            m_content.emplace(k, v->copy());
    }

    void StatsGrid::_set_lookup_table_if_relative(StatPtr& ptr) {
        if (ptr->type() != 2)
            return;

        auto& relative = dynamic_cast<RelativeStat&>(*ptr);
        relative.lookup_table(this);
    }
}
