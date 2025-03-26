#include "zzz/stats/regular.hpp"

//std
#include <memory>

//library
#include "library/format.hpp"

namespace zzz {
    StatPtr RegularStat::make(StatId id, Tag tag, bool conditional, double base) {
        return std::make_unique<RegularStat>(id, tag, conditional, base);
    }
    StatPtr RegularStat::make_from(const utl::json::Array& array) {
        switch (array.size()) {
        case 3:
            return make(
                (StatId) array[0].as_string(),
                Tag::Universal,
                array[1].as_bool(),
                array[2].as_floating()
            );

        case 4:
            return make(
                (StatId) array[0].as_string(),
                (Tag) array[1].as_string(),
                array[2].as_bool(),
                array[3].as_floating()
            );

        default:
            throw RUNTIME_ERROR("wrong arguments");
        }
    }

    RegularStat::RegularStat(StatId id, Tag tag, bool conditional, double base) :
        IStat(id, tag, conditional, base, 1) {
    }

    StatPtr RegularStat::copy() const {
        return std::make_unique<RegularStat>(m_unique.id, m_unique.tag, m_unique.conditional, m_base);
    }

    double RegularStat::value() const { return m_base; }

    void RegularStat::add(const StatPtr& another) {
        switch (another->type()) {
        case 1:
            m_base += another->base();
            break;

        case 2:
            throw RUNTIME_ERROR("TODO");

        default:
            throw RUNTIME_ERROR("wrong another.type()");
        }
    }
}
