#pragma once

//utl
#include "utl/json.hpp"

//zzz
#include "zzz/stats/basic.hpp"

namespace zzz {
    class RegularStat : public IStat {
    public:
        static StatPtr make(StatId id, Tag tag, double base);
        // [StatId (str), Tag (str, optional, Universal as default), is_conditional (bool), value (number)]
        // length is either 3 or 4
        static StatPtr make_from(const utl::Json& json, Tag tag);

        RegularStat(StatId id, Tag tag, double base);

        StatPtr copy() const override;

        double value() const override;

    protected:
        StatPtr add_as_copy(const StatPtr& another) override;
    };
}
