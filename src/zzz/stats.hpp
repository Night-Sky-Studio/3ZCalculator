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

#ifdef DEBUG_STATUS
#include "tabulate/table.hpp"
#endif

namespace zzz {
    struct stat {
        double value = 0;
        StatType type = StatType::None;
        Tag tag = Tag::Universal;

        operator double() const { return value; }
    };

    class ToStatConverter : public lib::IConverter<stat, toml::array>, public lib::IConverter<stat, toml::table> {
    public:
        stat from(const toml::array& data) const override;
        stat from(const toml::table& data) const override;
    };

    class StatsGrid {
        friend class ToStatsGridConverter;

    public:
        using iterator = std::unordered_map<size_t, stat>::iterator;
        using const_iterator = std::unordered_map<size_t, stat>::const_iterator;

        static stat no_value;

#ifdef DEBUG_STATUS
        tabulate::Table get_debug_table() const;
#endif

        stat get(StatType type) const;
        stat get_all(StatType type, Tag tag) const;
        stat get_only(StatType type, Tag tag) const;

        stat& at(StatType type, Tag tag = Tag::Universal);
        stat at(StatType type, Tag tag = Tag::Universal) const;

        bool emplace(stat what);

        void add(const StatsGrid& another);
        void add(stat s);

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

    private:
        std::unordered_map<size_t, stat> _content;

        static size_t _gen_key(StatType type, Tag tag);
    };

    class ToStatsGridConverter : public lib::IConverter<StatsGrid, toml::value> {
    public:
        StatsGrid from(const toml::value& data) const override;
    };
}

namespace global {
    static const zzz::ToStatConverter to_stat;
    static const zzz::ToStatsGridConverter to_stats_grid;
}
