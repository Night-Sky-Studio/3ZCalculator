#include "zzz/stats_grid.hpp"

//std
#include <ranges>
#include <stdexcept>

//lib
#include "library/format.hpp"

namespace zzz {
    size_t gen_key(StatId id, Tag tag) {
        return uint8_t(id) << 8 | size_t(tag);
    }

    StatsGrid::StatsGrid(const StatsGrid& another) {
        _copy_from(another);
    }
    StatsGrid& StatsGrid::operator=(const StatsGrid& another) {
        _copy_from(another);
        return *this;
    }

    StatsGrid::StatsGrid(StatsGrid&& another) noexcept :
        m_content(std::move(another.m_content)) {
    }
    StatsGrid& StatsGrid::operator=(StatsGrid&& another) noexcept {
        m_content = std::move(another.m_content);
        return *this;
    }

    StatsGrid StatsGrid::make_from(const utl::Json& json, Tag mandatory_tag) {
        if (!json.is_array())
            throw std::runtime_error("stats have to be serialized from json.array");

        StatsGrid result;

        for (const auto& it : json.as_array()) {
            auto stat = StatFactory::make(it);
            stat->m_tag = mandatory_tag;
            result.set(stat);
        }

        return result;
    }

#ifdef DEBUG_STATUS
    tabulate::Table StatsGrid::get_debug_table() const {
        tabulate::Table table;

        table.add_row({ "stat_type", "tag", "value" });

        for (const auto& stat : m_content | std::views::values) {
            auto value = lib::format("{:.4f}", stat->value());
            size_t end = value.find_last_not_of('0');
            if (end != std::string::npos) {
                value = value.substr(0, end + (value[end] == '.' ? 0 : 1));
            }

            table.add_row({
                convert::stat_id_to_string(stat->id()),
                convert::tag_to_string(stat->tag()),
                value
            });
        }

        return table;
    }
#endif

    double StatsGrid::get(StatId id, Tag tag) const {
        auto it = m_content.find(gen_key(id, tag));
        return it != m_content.end() ? it->second->value() : 0.0;
    }
    double StatsGrid::get_summed(StatId id, Tag tag) const {
        double result = 0.0;

        auto it = m_content.find(gen_key(id, Tag::Universal));
        if (it == m_content.end())
            return result;

        result += it->second->value();
        if (tag == Tag::Universal)
            return result;

        it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            return result;

        result += it->second->value();
        return result;
    }

    double& StatsGrid::at(StatId id, Tag tag) {
        auto it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            throw std::runtime_error(lib::format("key with id {} and tag {} is not found",
                convert::stat_id_to_string(id), convert::tag_to_string(tag)));
        return it->second->m_base;
    }
    const double& StatsGrid::at(StatId id, Tag tag) const {
        auto it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            throw std::runtime_error(lib::format("key with id {} and tag {} is not found",
                convert::stat_id_to_string(id), convert::tag_to_string(tag)));
        return it->second->m_base;
    }

    bool StatsGrid::contains(StatId id, Tag tag) const {
        return m_content.contains(gen_key(id, tag));
    }

    void StatsGrid::set(const StatPtr& stat) {
        size_t key = gen_key(stat->m_id, stat->m_tag);
        auto it = m_content.find(key);
        if (it != m_content.end())
            it->second = stat;
        else
            m_content.emplace(key, stat);
    }
    void StatsGrid::add(const StatPtr& stat) {
        size_t key = gen_key(stat->m_id, stat->m_tag);
        auto it = m_content.find(key);
        if (it != m_content.end())
            it->second->m_base += stat->m_base;
        else
            m_content.emplace(key, stat);
    }

    void StatsGrid::add_regular(double value, StatId id, Tag tag) {
        RegularStat on_emplace;

        on_emplace.m_base = value;
        on_emplace.m_id = id;
        on_emplace.m_tag = tag;

        return add(std::make_shared<IStat>(on_emplace));
    }
    void StatsGrid::add_relative(const std::string& formula, StatId id, Tag tag) {}

    void StatsGrid::add(const StatsGrid& stat) {
        for (const auto& [k, v] : stat.m_content) {
            auto it = m_content.find(k);
            if (it != m_content.end())
                it->second->m_base += v->m_base;
            else
                m_content.emplace(k, v->copy_as_ptr());
        }
    }

    //StatPtr StatsGrid::_no_value = RegularStat::make(0, StatId::None, Tag::Universal);

    void StatsGrid::_copy_from(const StatsGrid& another) {
        m_content.clear();
        for (const auto& [k, v] : another.m_content)
            m_content.emplace(k, v->copy_as_ptr());
    }
}
