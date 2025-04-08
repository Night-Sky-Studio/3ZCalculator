#pragma once

//boost
#include "boost/container/flat_map.hpp"

//utl
#include "utl/json.hpp"

//zzz
#include "zzz/stats/basic.hpp"

namespace boost {
    using namespace container;
}

namespace zzz {
    class StatsGrid {
    public:
        static StatPtr make_defined_relative_stat(StatId id, Tag tag);

        static StatsGrid make_from(const utl::Json& json, Tag tag = Tag::Universal);

        StatsGrid() = default;

        StatsGrid(const StatsGrid& another) noexcept;
        StatsGrid& operator=(const StatsGrid& another) noexcept;

        StatsGrid(StatsGrid&& another) noexcept;
        StatsGrid& operator=(StatsGrid&& another) noexcept;

        double get_value(qualifier_t key) const;
        // returns sum of your tag and universal tag
        double get_summed_value(qualifier_t key) const;

        // replaces ptr of stat if it exists
        void set(StatPtr&& value);

        bool contains(qualifier_t key) const;

        // emplaces element as regular stat with base 0.0 if it doesn't exist
        IStat& at(qualifier_t key);
        const IStat& at(qualifier_t key) const;

        // adds value of stat if it exists
        // otherwise emplaces it
        void add(const StatPtr& stat);

        // sums with other stats grid
        void add(const StatsGrid& another);

    protected:
        // map, because it's lightweight
        // and emplaces new elements faster than unordered_map
        //std::map<size_t, StatPtr> m_content;
        boost::flat_map<size_t, StatPtr> m_content;

    private:
        void _copy_from(const StatsGrid& another);

        void _set_lookup_table_if_relative(StatPtr& ptr);
    };
}
