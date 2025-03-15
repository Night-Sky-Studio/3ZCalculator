#include "zzz/stats.hpp"

//std
#include <ranges>
#include <stdexcept>

#ifdef DEBUG_STATUS
#include "fmt/format.h"
#include "tabulate/table.hpp"
#endif

namespace zzz {
    // StatAdaptor

    stat ToStatConverter::from(const toml::array& data) const {
        return stat {
            .value = data[1].as_floating(),
            .type = convert::string_to_stat_type(data[0].as_string()),
            .tag = data.size() == 3 ? convert::string_to_tag(data[2].as_string()) : Tag::Universal
        };
    }
    stat ToStatConverter::from(const toml::table& data) const {
        auto tag_it = data.find("tag");
        return stat {
            .value = data.at("value").as_floating(),
            .type = convert::string_to_stat_type(data.at("type").as_string()),
            .tag = tag_it != data.end() ? convert::string_to_tag(tag_it->second.as_string()) : Tag::Universal
        };
    }

    // StatsTable

    stat StatsGrid::no_value = { .value = 0.0, .type = StatType::None, .tag = Tag::Universal };

#ifdef DEBUG_STATUS
    tabulate::Table StatsGrid::get_debug_table() const {
        tabulate::Table table;

        table.add_row({ "stat_type", "tag", "value" });

        for (const auto& stat : _content | std::views::values) {
            auto value = fmt::vformat("{:.4f}", fmt::make_format_args(stat.value));
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

    //std::list<const stat*> StatsGrid::get_stats_by_tag(Tag tag) const { return _content_by_tag.at(tag); }

    StatsGrid::iterator StatsGrid::begin() { return _content.begin(); }
    StatsGrid::iterator StatsGrid::end() { return _content.end(); }

    StatsGrid::const_iterator StatsGrid::begin() const { return _content.cbegin(); }
    StatsGrid::const_iterator StatsGrid::end() const { return _content.cend(); }

    size_t StatsGrid::_gen_key(StatType type, Tag tag) {
        return (size_t) type | (size_t) tag << 8;
    }

    // StatsTableLoader

    StatsGrid ToStatsGridConverter::from(const toml::value& data) const {
        StatsGrid result;

        for (const auto& it : data.as_array()) {
            switch (it.type()) {
            case toml::value_t::array:
                result.emplace(global::to_stat.from(it.as_array()));
                break;
            case toml::value_t::table:
                result.emplace(global::to_stat.from(it.as_table()));
                break;
            default:
                throw std::runtime_error("wrong stat format");
            }
        }

        return result;
    }
}
