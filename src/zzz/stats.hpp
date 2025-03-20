#pragma once

//std
#include <list>
#include <unordered_map>

//utl
#include "utl/json.hpp"

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

    class StatsConverter {
    public:
        static stat to_stat_from(const utl::json::Node& source);

        static StatsGrid to_stats_grid_from(const utl::json::Node& source);
    };
}
