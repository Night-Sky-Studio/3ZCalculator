#include "zzz/stats.hpp"

//std
#include <algorithm>
#include <cmath>
#include <span>
#include <stack>

//lib
#include "library/format.hpp"
#include "library/string_funcs.hpp"
#include "stats_grid.hpp"

using namespace lib::rpn_parser;

namespace zzz::stats_details {
    optimized_rpn_t sum_rpns(
        const optimized_rpn_t& lhs, const
        optimized_rpn_t& rhs,
        lookup_token_t concatenation_token) {
        optimized_rpn_t result;

        result.reserve(lhs.size() + rhs.size() + 1);
        result.append_range(lhs);
        result.append_range(rhs);
        result.emplace_back(std::move(concatenation_token));

        return result;
    }
}

namespace zzz {
    std::map<char, stats_details::optimized_rpn_t> formulas_from_string(const std::string& line) {
        std::map<char, stats_details::optimized_rpn_t> result;

        auto parts = lib::split_as_view(line, ';');
        for (const auto& it : parts) {
            // divides on 2 parts: name and formula
            auto splitted = lib::split_as_copy(it, ':');

            // splitted[0][0] - first letter of formula
            char key = splitted[0][0];

            // splitted[1] - actual formula
            auto tokens = lib::RpnParser::tokenize(std::move(splitted[1]));
            auto rpn = lib::RpnParser::shunting_yard_algorithm(tokens);

            // converting strings to StatId
            stats_details::optimized_rpn_t optimized_rpn;
            optimized_rpn.reserve(rpn.size());
            for (const auto& it : rpn) {
                if (it.type == Variable)
                    optimized_rpn.emplace_back(it.type, (StatId) it.literal);
                else
                    optimized_rpn.emplace_back(it.type, it.number);
            }

            result.emplace(key, std::move(optimized_rpn));
        }

        return result;
    }

    double eval_rpn(const stats_details::optimized_rpn_t& rpn, const StatsGrid& lookup_table) {
        std::stack<double> stack;
        auto pop_and_get = [&stack] {
            double val = stack.top();
            stack.pop();
            return val;
        };

        for (const auto& it : rpn) {
            switch (it.type) {
            case Number:
                stack.emplace(std::get<double>(it.value));
                continue;

            case Variable:
                stack.emplace(lookup_table.get(std::get<StatId>(it.value)));
                continue;
            }

            double rhs = pop_and_get();
            double lhs = pop_and_get();

            switch (it.type) {
            case Plus:
                stack.emplace(lhs + rhs);
                break;

            case Minus:
                stack.emplace(lhs - rhs);
                break;

            case Star:
                stack.emplace(lhs * rhs);
                break;

            case Slash:
                stack.emplace(lhs / rhs);
                break;

            case Percent:
                stack.emplace(std::fmod(lhs, rhs));
                break;

            case Less:
                stack.emplace(lhs < rhs);
                break;

            case More:
                stack.emplace(lhs > rhs);
                break;

            case LessEq:
                stack.emplace(lhs <= rhs);
                break;

            case MoreEq:
                stack.emplace(lhs >= rhs);
                break;

            default:
                throw RUNTIME_ERROR("rpn eval wrong token");
            }
        }

        return stack.top();
    }
}

namespace zzz {
    // IStat

    IStat::IStat(size_t type) :
        _type(type) {
    }

    double IStat::base() const {
        return m_base;
    }
    StatId IStat::id() const {
        return m_id;
    }
    Tag IStat::tag() const {
        return m_tag;
    }

    // RegularStat

    StatPtr RegularStat::make(double base, StatId id, Tag tag) {
        RegularStat result;

        result.m_base = base;
        result.m_id = id;
        result.m_tag = tag;

        return std::make_shared<RegularStat>(std::move(result));
    }

    StatPtr RegularStat::make_from_floating(const utl::Json& json, StatId id) {
        return make(json.as_floating(), id, Tag::Universal);
    }
    StatPtr RegularStat::make_from_object(const utl::Json& json, StatId id, Tag tag) {
        return make(json["val"].as_floating(), id, tag);
    }

    RegularStat::RegularStat() :
        IStat(1) {
    }

    StatPtr RegularStat::copy_as_ptr() const {
        RegularStat result;

        result.m_base = m_base;
        result.m_id = m_id;
        result.m_tag = m_tag;

        return std::make_shared<RegularStat>(std::move(result));
    }

    double RegularStat::value() const {
        return m_base;
    }

    // RelativeStat

    StatPtr RelativeStat::make(double base, formulas_t formulas, StatId id, Tag tag) {
        RelativeStat result;

        result.m_base = base;
        result.m_id = id;
        result.m_tag = Tag::Universal;
        result.m_formulas = std::move(formulas);

        return std::make_shared<RelativeStat>(std::move(result));
    }
    StatPtr RelativeStat::make(double base, const std::string& formula, StatId id, Tag tag) {
        return make(base, formulas_from_string(formula), id, tag);
    }

    StatPtr RelativeStat::make_from_string(const utl::Json& json, StatId key) {
        return make(0.0, json.as_string(), key, Tag::Universal);
    }
    StatPtr RelativeStat::make_from_object(const utl::Json& json, StatId key, Tag tag) {
        return make(json.value_or<double>("val", 0.0), json["formulas"].as_string(), key, tag);
    }

    RelativeStat::RelativeStat() :
        IStat(2) {
    }

    StatPtr RelativeStat::copy_as_ptr() const {
        RelativeStat result;

        result.m_base = m_base;
        result.m_id = m_id;
        result.m_tag = m_tag;
        result.m_formulas = m_formulas;

        return std::make_shared<RelativeStat>(std::move(result));
    }

    double RelativeStat::value() const {
        if (!m_lookup_table)
            throw RUNTIME_ERROR("you have to specify lookup_table first");

        if (auto cond_rpn = m_formulas.find('c'); cond_rpn != m_formulas.end()
            && !eval_rpn(cond_rpn->second, *m_lookup_table))
            return m_base;

        const auto& func_rpn = m_formulas.at('f');
        double calculated = eval_rpn(func_rpn, *m_lookup_table);

        if (auto max_rpn = m_formulas.find('m'); max_rpn != m_formulas.end()) {
            double max = eval_rpn(max_rpn->second, *m_lookup_table);
            calculated = std::min(calculated, max);
        }

        return m_base + calculated;
    }
    const RelativeStat::formulas_t& RelativeStat::formulas() { return m_formulas; }

    void RelativeStat::set_lookup_table(const StatsGrid& lookup_table) {
        m_lookup_table = &lookup_table;
    }

    // StatFactory

    std::list<std::tuple<size_t, StatFactory::condition_t>> StatFactory::m_list_of_conditions;

    StatFactory::condition_t::condition_t(std::function<size_t(const utl::Json&)> f) :
        checker(std::move(f)) {
    }
    size_t StatFactory::condition_t::operator()(const utl::Json& json) const {
        return checker(json);
    }

    void StatFactory::init_default() {
        // regular floating
        m_list_of_conditions.emplace_back(15, condition_t(
            [](const utl::Json& json) -> size_t { return json.is_floating(); }
        ));
        // relative string
        m_list_of_conditions.emplace_back(23, condition_t(
            [](const utl::Json& json) -> size_t { return json.is_string(); }
        ));
        // regular object
        m_list_of_conditions.emplace_back(11, condition_t(
            [](const utl::Json& json) -> size_t {
                if (!json.is_object())
                    return 0;
                const auto& table = json.as_object();
                auto val_it = table.find("val");

                if (val_it == table.end() || !val_it->second.is_floating())
                    return 0;

                if (auto tag_it = table.find("tags"); tag_it != table.end())
                    return tag_it->second.as_array().size();

                return 1;
            }
        ));
        // relative object (when we have val)
        m_list_of_conditions.emplace_back(210, condition_t(
            [](const utl::Json& json) -> size_t {
                if (!json.is_object())
                    return 0;
                const auto& table = json.as_object();

                auto val_it = table.find("val");
                if (val_it == table.end() || !val_it->second.is_string())
                    return 0;

                if (auto tag_it = table.find("tags"); tag_it != table.end())
                    return tag_it->second.as_array().size();

                return 1;
            }
        ));
        // relative object (when we have base)
        m_list_of_conditions.emplace_back(211, condition_t(
            [](const utl::Json& json) -> size_t {
                if (!json.is_object())
                    return 0;
                const auto& table = json.as_object();

                if (!json.contains("val") || json.contains("formulas"))
                    return 0;

                if (auto tag_it = table.find("tags"); tag_it != table.end())
                    return tag_it->second.as_array().size();

                return 1;
            }
        ));
    }

    // TODO: make table of conditions more efficient
    std::vector<StatPtr> StatFactory::make(const std::string& key, const utl::Json& json) {
        size_t size = 0, id = 0;

        for (const auto& [k, f] : m_list_of_conditions)
            if ((size = f(json))) {
                id = k;
                break;
            }

        if (!id || !size)
            throw RUNTIME_ERROR("can't define what stat you have");

        switch (id) {
        case 15:
            return { RegularStat::make_from_floating(json, (StatId) key) };

        case 23:
            return { RelativeStat::make_from_string(json, (StatId) key) };

        case 11: {
            if (size == 1)
                return {
                    RegularStat::make_from_object(json, (StatId) key,
                        (Tag) json.value_or<std::string>("tag", "universal"))
                };

            std::vector<StatPtr> result;
            result.reserve(size);

            const auto& tags = json["tags"].as_array();
            for (size_t i = 0; i < size; i++)
                result.emplace_back(RegularStat::make_from_object(json, (StatId) key, (Tag) tags[i].as_string()));

            return result;
        }

        case 210: {
            if (size == 1)
                return {
                    RelativeStat::make_from_object(json, (StatId) key,
                        (Tag) json.value_or<std::string>("tag", "universal"))
                };

            std::vector<StatPtr> result;
            result.reserve(size);

            const auto& tags = json["tags"].as_array();
            for (size_t i = 0; i < size; i++)
                result.emplace_back(RelativeStat::make_from_object(json, (StatId) key, (Tag) tags[i].as_string()));

            return result;
        }

        case 211: {
            if (size == 1)
                return {
                    RelativeStat::make_from_object(json, (StatId) key,
                        (Tag) json.value_or<std::string>("tag", "universal"))
                };

            std::vector<StatPtr> result;
            result.reserve(size);

            const auto& tags = json["tags"].as_array();
            for (size_t i = 0; i < size; i++)
                result.emplace_back(RelativeStat::make_from_object(json, (StatId) key, (Tag) tags[i].as_string()));

            return result;
        }

        default:
            throw RUNTIME_ERROR("can't define what stat you have");
        }
    }
}
