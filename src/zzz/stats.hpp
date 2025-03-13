#pragma once

//std
#include <list>
#include <unordered_map>

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"

//zzz
#include "zzz/enums.hpp"

namespace zzz {
    struct stat {
        double value = 0;
        StatType type = StatType::None;
        Tag tag = Tag::Universal;

        operator double() const { return value; }
    };

    class ToStatConverter : public lib::IConverter<stat, toml::array>, public lib::IConverter<stat, toml::table> {
    public:
        stat from(const toml::array& data) override;
        stat from(const toml::table& data) override;
    };

    class StatsGrid {
        friend class ToStatsGridConverter;

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

    class ToStatsGridConverter : public lib::IConverter<StatsGrid, toml::value> {
    public:
        StatsGrid from(const toml::value& data) override;
    };
}

namespace zzz::global {
    static ToStatConverter to_stat;

    static ToStatsGridConverter to_stats_grid;
}
