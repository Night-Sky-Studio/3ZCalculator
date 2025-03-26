#pragma once

//std
#include <map>
#include <variant>
#include <vector>

//utl
#include "utl/json.hpp"

//library
#include "library/rpn.hpp"

//zzz
#include "zzz/stats/basic.hpp"
#include "zzz/stats/grid.hpp"

namespace zzz {
    class StatToken {
    public:
        template<typename T>
            requires(std::is_same_v<T, double> || std::is_same_v<T, StatId>)
        StatToken(lib::rpn_token_type type, T value) :
            _type(type),
            _value(value) {
        }

        lib::rpn_token_type type() const { return _type; }

        double number() const { return std::get<double>(_value); }
        StatId variable() const { return std::get<StatId>(_value); }

    private:
        lib::rpn_token_type _type;
        std::variant<double, StatId> _value;
    };
    using stat_rpn_t = std::vector<StatToken>;
    using formulas_t = std::map<char, stat_rpn_t>;

    class RelativeStat : public IStat {
    public:
        static StatPtr make(StatId id, const Tag& tag, bool conditional, double base, const std::string& formulas);
        static StatPtr make(StatId id, const Tag& tag, bool conditional, double base, formulas_t formulas);
        static StatPtr make_from(const utl::json::Array& array);

        RelativeStat(StatId id, const Tag& tag, bool conditional, double base, const std::string& formulas);
        RelativeStat(StatId id, const Tag& tag, bool conditional, double base, formulas_t formulas);
        RelativeStat(const RelativeStat& another);

        StatPtr copy() const override;

        double value() const override;

        const formulas_t& formulas() const;

        void lookup_table(const StatsGrid& stats);

    protected:
        formulas_t m_formulas;
        const StatsGrid* m_lookup_table = nullptr;

        StatPtr add_as_copy(const StatPtr& another) override;
    };
}
