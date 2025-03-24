#pragma once

//std
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

//utl
#include "utl/json.hpp"

//library
#include "library/rpn.hpp"

//zzz
#include "zzz/enums.hpp"

namespace zzz::stats_details {
    struct lookup_token_t {
        lib::rpn_parser::TokenType type;
        std::variant<double, StatId> value;
    };
    using optimized_rpn_t = std::vector<lookup_token_t>;

    optimized_rpn_t sum_rpns(
        const optimized_rpn_t& lhs,
        const optimized_rpn_t& rhs,
        lookup_token_t concatenation_token);
}

namespace zzz {
    class StatsGrid;
    class IStat;

    using StatPtr = std::shared_ptr<IStat>;

    class IStat {
        friend class StatsGrid;

    public:
        explicit IStat(size_t type);
        virtual ~IStat() = default;

        virtual StatPtr copy_as_ptr() const = 0;

        virtual double value() const = 0;

        double base() const;
        StatId id() const;
        Tag tag() const;

    protected:
        double m_base = 0;
        StatId m_id = StatId::None;
        Tag m_tag = Tag::Universal;

    private:
        size_t _type;
    };

    class RegularStat : public IStat {
    public:
        static StatPtr make(double base, StatId id, Tag tag);

        static StatPtr make_from_floating(const utl::Json& json, StatId id);
        static StatPtr make_from_object(const utl::Json& json, StatId id, Tag tag);

        explicit RegularStat();

        StatPtr copy_as_ptr() const override;

        // m_base can be changed only by adding stats with each other
        double value() const override;
    };

    // m_base is zero and can be changed only by summing with RegularStats
    class RelativeStat : public IStat {
    public:
        using formulas_t = std::map<char, stats_details::optimized_rpn_t>;

        static StatPtr make(double base, formulas_t formulas, StatId id, Tag tag);
        static StatPtr make(double base, const std::string& formula, StatId id, Tag tag);

        static StatPtr make_from_string(const utl::Json& json, StatId key);
        static StatPtr make_from_object(const utl::Json& json, StatId key, Tag tag);

        explicit RelativeStat();

        StatPtr copy_as_ptr() const override;
        // to get value, you have to specify lookup_table
        double value() const override;
        const formulas_t& formulas();

        void set_lookup_table(const StatsGrid& lookup_table);

    protected:
        // ['c'] - condition, ['f'] - function, ['m'] - max
        formulas_t m_formulas;
        const StatsGrid* m_lookup_table = nullptr;
    };

    class StatFactory {
        struct condition_t {
            explicit condition_t(std::function<size_t(const utl::Json&)> f);

            std::function<size_t(const utl::Json&)> checker;

            size_t operator()(const utl::Json& json) const;
        };

    public:
        static void init_default();

        static std::vector<StatPtr> make(const std::string& key, const utl::Json& json);

    protected:
        static std::list<std::tuple<size_t, condition_t>> m_list_of_conditions;
    };
}
