#include "zzz/stats.hpp"

//std
#include <ranges>
#include <stdexcept>

#ifdef DEBUG_STATUS
#include "library/format.hpp"
#include "tabulate/table.hpp"
#endif

namespace zzz {
    stat StatsGrid::no_value = { .value = 0.0, .type = StatType::None, .tag = Tag::Universal };

#ifdef DEBUG_STATUS
    tabulate::Table StatsGrid::get_debug_table() const {
        tabulate::Table table;

        table.add_row({ "stat_type", "tag", "value" });

        for (const auto& stat : _content | std::views::values) {
            auto value = lib::format("{:.4f}", stat.value);
            size_t end = value.find_last_not_of('0');
            if (end != std::string::npos) {
                value = value.substr(0, end + (value[end] == '.' ? 0 : 1));
            }

            table.add_row({
                convert::stat_type_to_string(stat.type),
                convert::tag_to_string(stat.tag),
                value
            });
        }

        return table;
    }
#endif

    stat StatsGrid::get(StatType type) const {
        auto it = _content.find(_gen_key(type, Tag::Universal));
        return it != _content.end() ? it->second : no_value;
    }
    stat StatsGrid::get_all(StatType type, Tag tag) const {
        auto universal_it = _content.find(_gen_key(type, Tag::Universal));
        auto result = universal_it != _content.end() ? universal_it->second : no_value;

        if (tag != Tag::Universal) {
            const auto tagged_it = _content.find(_gen_key(type, tag));
            result.value += tagged_it != _content.end() ? tagged_it->second.value : 0.0;
        }

        return result;
    }
    stat StatsGrid::get_only(StatType type, Tag tag) const {
        auto it = _content.find(_gen_key(type, tag));
        return it != _content.end() ? it->second : no_value;
    }

    stat& StatsGrid::at(StatType type, Tag tag) {
        auto key = _gen_key(type, tag);
        auto it = _content.find(key);

        if (it == _content.end())
            it = _content.emplace(key, stat { .value = 0.0, .type = type, .tag = tag }).first;

        return it->second;
    }
    stat StatsGrid::at(StatType type, Tag tag) const {
        auto it = _content.find(_gen_key(type, tag));
        return it != _content.end() ? it->second : stat {};
    }

    bool StatsGrid::emplace(stat what) {
        auto key = _gen_key(what.type, what.tag);
        auto [it, flag] = _content.emplace(key, what);

        return flag;
    }

    void StatsGrid::add(const StatsGrid& another) {
        for (const auto& it : another | std::views::values) {
            auto key = _gen_key(it.type, it.tag);
            if (auto jt = _content.find(key); jt != _content.end())
                jt->second.value += it.value;
            else
                _content.emplace(key, it);
        }
    }
    void StatsGrid::add(stat s) {
        auto key = _gen_key(s.type, s.tag);
        if (auto jt = _content.find(key); jt != _content.end())
            jt->second.value += s.value;
        else
            _content.emplace(key, s);
    }

    StatsGrid::iterator StatsGrid::begin() { return _content.begin(); }
    StatsGrid::iterator StatsGrid::end() { return _content.end(); }

    StatsGrid::const_iterator StatsGrid::begin() const { return _content.cbegin(); }
    StatsGrid::const_iterator StatsGrid::end() const { return _content.cend(); }

    size_t StatsGrid::_gen_key(StatType type, Tag tag) {
        return (size_t) type | (size_t) tag << 8;
    }

    // ToStatsConverter

    inline stat to_stat_from_object(const utl::json::Node& source) {
    }

    stat StatsConverter::to_stat_from(const utl::json::Node& source) {
        
    }
    StatsGrid StatsConverter::to_stats_grid_from(const utl::json::Node& source) {
    }
}
