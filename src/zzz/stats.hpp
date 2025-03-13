#pragma once

//std
#include <cmath>
#include <list>
#include <unordered_map>

//toml11
#include "toml.hpp"

//library
#include "library/adaptor.hpp"

//zzz
#include "zzz/enums.hpp"

namespace zzz {
    struct stat {
        double value = 0;
        StatType type = StatType::None;
        Tag tag = Tag::Universal;

        operator double() const { return value; }
    };

    class StatAsArrayAdaptor : public lib::IAdaptor<toml::array, stat> {
    public:
        toml::array to_t1(const stat& data) override;
        stat to_t2(const toml::array& data) override;
    };
    class StatAsObjectAdaptor : public lib::IAdaptor<toml::table, stat> {
    public:
        toml::table to_t1(const stat& data) override;
        stat to_t2(const toml::table& data) override;
    };

    class StatsGrid {
        friend class StatsGridAdaptor;

    public:
        using iterator = std::unordered_map<size_t, stat>::iterator;
        using const_iterator = std::unordered_map<size_t, stat>::const_iterator;

        static stat no_value;

        stat get(StatType type) const;
        stat get_all(StatType type, Tag tag) const;
        stat get_only(StatType type, Tag tag) const;

        stat& at(StatType type, Tag tag = Tag::Universal);
        stat at(StatType type, Tag tag = Tag::Universal) const;

        bool emplace(stat what);

        void add(const StatsGrid& another);
        void add(stat s);

        std::list<const stat*> get_stats_by_tag(Tag tag) const;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

    private:
        std::unordered_map<size_t, stat> _content;
        std::unordered_map<Tag, std::list<const stat*>> _content_by_tag;

        static size_t _gen_key(StatType type, Tag tag);
    };

    class StatsGridAdaptor : public lib::IAdaptor<toml::value, StatsGrid> {
    public:
        toml::value to_t1(const StatsGrid& data) override;
        StatsGrid to_t2(const toml::value& data) override;
    };
}

namespace zzz::global {
    static StatAsArrayAdaptor stat_as_array_adaptor;
    static StatAsObjectAdaptor stat_as_object_adaptor;

    static StatsGridAdaptor stats_grid_adaptor;
}
