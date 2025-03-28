#pragma once

//std
#include <memory>

//zzz
#include "zzz/enums.hpp"

namespace zzz {
    using StatPtr = std::unique_ptr<class IStat>;

    struct qualifier_t {
        StatId id;
        Tag tag;

        size_t hash() const { return size_t(id) | size_t(tag) << 8; }
    };

    class IStat {
        friend class StatsGrid;

    public:
        virtual ~IStat() = default;

        virtual StatPtr copy() const = 0;

        qualifier_t qualifier() const { return m_unique; }

        double& base() { return m_base; }
        const double& base() const { return m_base; }

        size_t type() const { return _type; }

        virtual double value() const = 0;

    protected:
        // acts like identifier in grid
        const qualifier_t m_unique;
        double m_base;

        IStat(StatId id, Tag tag, double base, size_t type) :
            m_unique({ .id = id, .tag = tag }),
            m_base(base),
            _type(type) {
        }

        // assume that this->m_unique == another.m_unique
        virtual StatPtr add_as_copy(const StatPtr& another) = 0;

    private:
        size_t _type;
    };
}
