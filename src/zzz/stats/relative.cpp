#include "zzz/stats/relative.hpp"

//std
#include <algorithm>
#include <ranges>
#include <stack>

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"
#include "library/template_math.hpp"

using enum lib::rpn_parser::TokenType;

namespace zzz {
    auto make_formulas(const std::string& source) {
        formulas_t result;

        for (const auto& it : lib::split_as_view(source, ';')) {
            // divides on 2 parts: name and formula
            auto splitted = lib::split_as_copy(it, ':');

            // splitted[1] - actual formula
            auto tokens = lib::RpnParser::tokenize(std::move(splitted[1]));
            auto rpn = lib::RpnParser::shunting_yard_algorithm(tokens);

            // converting strings to StatId
            stat_rpn_t temp;
            temp.reserve(rpn.size());
            for (const auto& [type, literal, number] : rpn) {
                switch (type) {
                case Variable:
                    temp.emplace_back(type, (StatId) literal);
                    break;

                default:
                    // for operators and numbers
                    temp.emplace_back(type, number);
                }
            }

            // splitted[0][0] - first letter of formula name which can be used as identifier
            result.emplace(splitted[0][0], std::move(temp));
        }

        return result;
    }

    double eval(const stat_rpn_t& rpn, const StatsGrid& variables) {
        std::stack<double> stack;

        for (const auto& it : rpn) {
            if (it.type() == Number) {
                stack.emplace(it.number());
                continue;
            }

            if (it.type() == Variable) {
                stack.emplace(variables.get_value({
                    .id = it.variable(),
                    .tag = Tag::Universal,
                    .conditional = false
                }));
                continue;
            }

            double rhs = stack.top();
            stack.pop();
            double lhs = stack.top();
            stack.pop();

            auto op = lib::math_ops.find(it.type());
            if (op == lib::math_ops.end())
                throw RUNTIME_ERROR("wrong rpn token");

            stack.emplace(op->second(lhs, rhs));
        }

        if (stack.size() > 1)
            throw FMT_RUNTIME_ERROR("remaining stack size after rpn eval is {}", stack.size());

        return stack.top();
    }
}

namespace zzz {
    StatPtr RelativeStat::make(StatId id, const Tag& tag, bool conditional, double base, formulas_t formulas) {
        return std::make_unique<RelativeStat>(id, tag, conditional, base, std::move(formulas));
    }
    StatPtr RelativeStat::make(StatId id, const Tag& tag, bool conditional, double base, const std::string& formulas) {
        return make(id, tag, conditional, base, make_formulas(formulas));
    }
    StatPtr RelativeStat::make_from(const utl::json::Array& array) {
        switch (array.size()) {
        case 4:
            return make(
                (StatId) array[0].as_string(),
                Tag::Universal,
                array[1].as_bool(),
                array[2].as_floating(),
                array[3].as_string()
            );

        case 5:
            return make(
                (StatId) array[0].as_string(),
                (Tag) array[1].as_string(),
                array[2].as_bool(),
                array[3].as_floating(),
                array[4].as_string()
            );

        default:
            throw RUNTIME_ERROR("wrong arguments");
        }
    }

    RelativeStat::RelativeStat(StatId id, const Tag& tag, bool conditional, double base, formulas_t formulas) :
        IStat(id, tag, conditional, base, 2),
        m_formulas(std::move(formulas)) {
    }
    RelativeStat::RelativeStat(StatId id, const Tag& tag, bool conditional, double base, const std::string& formulas) :
        RelativeStat(id, tag, conditional, base, make_formulas(formulas)) {
    }
    RelativeStat::RelativeStat(const RelativeStat& another) :
        IStat(another.m_unique.id, another.m_unique.tag, another.m_unique.conditional, another.m_base, 2),
        m_formulas(another.m_formulas),
        m_lookup_table(another.m_lookup_table) {
    }

    StatPtr RelativeStat::copy() const {
        return std::make_unique<RelativeStat>(*this);
    }

    double RelativeStat::value() const {
        if (!m_lookup_table)
            throw RUNTIME_ERROR("you have to specify lookup_table first");

        if (auto cond_rpn = m_formulas.find('c'); cond_rpn != m_formulas.end()
            && !eval(cond_rpn->second, *m_lookup_table))
            return m_base;

        const auto& func_rpn = m_formulas.at('f');
        double calculated = eval(func_rpn, *m_lookup_table);

        if (auto max_rpn = m_formulas.find('m'); max_rpn != m_formulas.end()) {
            double max = eval(max_rpn->second, *m_lookup_table);
            calculated = std::min(calculated, max);
        }

        return m_base + calculated;
    }

    const formulas_t& RelativeStat::formulas() const { return m_formulas; }

    void RelativeStat::lookup_table(const StatsGrid& stats) { m_lookup_table = &stats; }

    StatPtr RelativeStat::add_as_copy(const StatPtr& another) {
        StatPtr result;

        switch (another->type()) {
        case 1:
            result = make(
                m_unique.id,
                m_unique.tag,
                m_unique.conditional,
                m_base + another->base(),
                m_formulas
            );
            break;

        case 2:
            throw RUNTIME_ERROR("TODO");

        default:
            throw RUNTIME_ERROR("wrong another.type()");
        }

        return result;
    }
}
