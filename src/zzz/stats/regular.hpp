#pragma once

//utl
#include "utl/json.hpp"

//zzz
#include "zzz/stats/basic.hpp"

namespace zzz {
    class RegularStat : public IStat {
    public:
        static StatPtr make(StatId id, Tag tag, bool conditional, double base);
        // [StatId (str), Tag (str, optional, Universal as default), is_conditional (bool), value (number)]
        // length is either 3 or 4
        static StatPtr make_from(const utl::json::Array& array);

        RegularStat(StatId id, Tag tag, bool conditional, double base);

        StatPtr copy() const override;

        double value() const override;

    protected:
        void add(const StatPtr& another) override;
    };
}
