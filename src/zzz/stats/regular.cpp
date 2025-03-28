#include "zzz/stats/regular.hpp"

//std
#include <memory>

//library
#include "library/format.hpp"

//zzz
#include "zzz/stats/relative.hpp"

namespace zzz {
    StatPtr RegularStat::make(StatId id, Tag tag, double base) {
        return std::make_unique<RegularStat>(id, tag, base);
    }
    StatPtr RegularStat::make_from(const utl::Json& json, Tag tag) {
        const auto& as_array = json.as_array();

        switch (as_array.size()) {
        case 2:
            return make(
                (StatId) json[0].as_string(),
                tag,
                json[2].as_floating()
            );

        case 3:
            return make(
                (StatId) json[0].as_string(),
                (Tag) json[1].as_string(),
                json[3].as_floating()
            );

        default:
            throw RUNTIME_ERROR("wrong arguments");
        }
    }

    RegularStat::RegularStat(StatId id, Tag tag, double base) :
        IStat(id, tag, base, 1) {
    }

    StatPtr RegularStat::copy() const {
        return std::make_unique<RegularStat>(*this);
    }

    double RegularStat::value() const { return m_base; }

    StatPtr RegularStat::add_as_copy(const StatPtr& another) {
        StatPtr result;

        switch (another->type()) {
        case 1:
            result = make(
                m_unique.id,
                m_unique.tag,
                m_base + another->base()
            );
            break;

        case 2: {
            auto ptr = dynamic_cast<const RelativeStat&>(*another);
            result = RelativeStat::make(
                m_unique.id,
                m_unique.tag,
                m_base + ptr.base(),
                ptr.formulas()
            );
            break;
        }

        default:
            throw RUNTIME_ERROR("wrong another.type()");
        }

        return result;
    }
}
