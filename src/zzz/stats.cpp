#include "zzz/stats.hpp"

//std
#include <ranges>
#include <stdexcept>

namespace zzz {
    // StatAdaptor

    toml::array StatAsArrayAdaptor::to_t1(const stat& data) {
        auto result = toml::array { convert::stat_type_to_string(data.type), data.value };

        if (data.tag != Tag::Universal)
            result.emplace_back(convert::tag_to_string(data.tag));

        return result;
    }
    stat StatAsArrayAdaptor::to_t2(const toml::array& data) {
        return stat {
            .value = data[1].as_floating(),
            .type = convert::string_to_stat_type(data[0].as_string()),
            .tag = data.size() == 3 ? convert::string_to_tag(data[2].as_string()) : Tag::Universal
        };
    }

    toml::table StatAsObjectAdaptor::to_t1(const stat& data) {
        auto result = toml::table { { "type", convert::stat_type_to_string(data.type) }, { "value", data.value } };

        if (data.tag != Tag::Universal)
            result.emplace("tag", convert::tag_to_string(data.tag));

        return result;
    }
    stat StatAsObjectAdaptor::to_t2(const toml::table& data) {
        auto tag_it = data.find("tag");
        return stat {
            .value = data.at("value").as_floating(),
            .type = convert::string_to_stat_type(data.at("type").as_string()),
            .tag = tag_it != data.end() ? convert::string_to_tag(tag_it->second.as_string()) : Tag::Universal
        };
    }

    // StatsTable

    stat StatsGrid::no_value = { .value = 0.0, .type = StatType::None, .tag = Tag::Universal };

    stat StatsGrid::get(StatType type) const {
        const auto it = _content.find(_gen_key(type, Tag::Universal));
        return it != _content.end() ? it->second : no_value;
    }
    stat StatsGrid::get_all(StatType type, Tag tag) const {
        const auto universal_it = _content.find(_gen_key(type, Tag::Universal));
        auto result = universal_it != _content.end() ? universal_it->second : no_value;

        if (tag != Tag::Universal) {
            const auto tagged_it = _content.find(_gen_key(type, tag));
            result.value += tagged_it != _content.end() ? tagged_it->second.value : 0.0;
        }

        return result;
    }
    stat StatsGrid::get_only(StatType type, Tag tag) const {
        const auto it = _content.find(_gen_key(type, tag));
        return it != _content.end() ? it->second : no_value;
    }

    stat& StatsGrid::at(StatType type, Tag tag) {
        return _content.at(_gen_key(type, tag));
    }
    stat StatsGrid::at(StatType type, Tag tag) const {
        const auto it = _content.find(_gen_key(type, tag));
        return it != _content.end() ? it->second : stat {};
    }

    bool StatsGrid::emplace(stat what) {
        auto key = _gen_key(what.type, what.tag);
        auto [it, flag] = _content.emplace(key, what);

        if (flag)
            _content_by_tag.at(it->second.tag).emplace_back(&it->second);

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

    std::list<const stat*> StatsGrid::get_stats_by_tag(Tag tag) const { return _content_by_tag.at(tag); }

    StatsGrid::iterator StatsGrid::begin() { return _content.begin(); }
    StatsGrid::iterator StatsGrid::end() { return _content.end(); }

    StatsGrid::const_iterator StatsGrid::begin() const { return _content.cbegin(); }
    StatsGrid::const_iterator StatsGrid::end() const { return _content.cend(); }

    size_t StatsGrid::_gen_key(StatType type, Tag tag) {
        return (size_t) type | (size_t) tag << 8;
    }

    // StatsTableLoader

    // TODO
    toml::value StatsGridAdaptor::to_t1(const StatsGrid& data) {
        return {};
    }
    StatsGrid StatsGridAdaptor::to_t2(const toml::value& data) {
        StatsGrid result;

        for (const auto& it : data.as_array()) {
            switch (it.type()) {
            case toml::value_t::array:
                result.emplace(global::stat_as_array_adaptor.to_t2(it.as_array()));
                break;
            case toml::value_t::table:
                result.emplace(global::stat_as_object_adaptor.to_t2(it.as_table()));
                break;
            default:
                throw std::runtime_error("wrong stat format");
            }
        }

        return result;
    }
}
