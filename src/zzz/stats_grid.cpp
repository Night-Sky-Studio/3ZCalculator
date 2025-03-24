#include "zzz/stats_grid.hpp"

//std
#include <ranges>
#include <stdexcept>

//lib
#include "library/format.hpp"

using namespace zzz::stats_details;

namespace zzz {
    size_t gen_key(StatId id, Tag tag) {
        return uint8_t(id) << 8 | size_t(tag);
    }

    StatsGrid::StatsGrid(const StatsGrid& another) {
        _copy_from(another);
    }
    StatsGrid& StatsGrid::operator=(const StatsGrid& another) noexcept {
        _copy_from(another);
        return *this;
    }

    StatsGrid::StatsGrid(StatsGrid&& another) noexcept :
        m_content(std::move(another.m_content)) {
    }
    StatsGrid& StatsGrid::operator=(StatsGrid&& another) noexcept {
        m_content = std::move(another.m_content);
        return *this;
    }

    StatsGrid StatsGrid::make_from(const utl::Json& json, Tag mandatory_tag) {
        if (!json.is_object())
            throw RUNTIME_ERROR("stats have to be serialized from json.object");

        StatsGrid result;

        for (const auto& [k, v] : json.as_object()) {
            auto stat = StatFactory::make(k, v);
            stat->m_tag = mandatory_tag;
            result.set(stat);
        }

        return result;
    }

#ifdef DEBUG_STATUS
    tabulate::Table StatsGrid::get_debug_table() const {
        tabulate::Table table;

        table.add_row({ "stat_type", "tag", "value" });

        for (const auto& stat : m_content | std::views::values) {
            auto value = lib::format("{:.4f}", stat->value());
            size_t end = value.find_last_not_of('0');
            if (end != std::string::npos) {
                value = value.substr(0, end + (value[end] == '.' ? 0 : 1));
            }

            table.add_row({
                (std::string_view) stat->id(),
                (std::string_view) stat->tag(),
                value
            });
        }

        return table;
    }
#endif

    double StatsGrid::get(StatId id, Tag tag) const {
        auto it = m_content.find(gen_key(id, tag));
        return it != m_content.end() ? it->second->value() : 0.0;
    }
    double StatsGrid::get_summed(StatId id, Tag tag) const {
        double result = 0.0;

        auto it = m_content.find(gen_key(id, Tag::Universal));
        if (it == m_content.end())
            return result;

        result += it->second->value();
        if (tag == Tag::Universal)
            return result;

        it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            return result;

        result += it->second->value();
        return result;
    }

    double& StatsGrid::at(StatId id, Tag tag) {
        auto it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            it = m_content.emplace(gen_key(id, tag), RegularStat::make(0, id, tag)).first;
        return it->second->m_base;
    }
    const double& StatsGrid::at(StatId id, Tag tag) const {
        auto it = m_content.find(gen_key(id, tag));
        if (it == m_content.end())
            throw FMT_RUNTIME_ERROR("key with id {} and tag {} is not found",
                (std::string_view)id, (std::string_view) tag);
        return it->second->m_base;
    }

    bool StatsGrid::contains(StatId id, Tag tag) const {
        return m_content.contains(gen_key(id, tag));
    }

    void StatsGrid::set(const StatPtr& stat) {
        size_t key = gen_key(stat->m_id, stat->m_tag);
        auto it = m_content.find(key);
        if (it != m_content.end())
            it->second = stat;
        else
            m_content.emplace(key, stat);
    }
    void StatsGrid::add(const StatPtr& stat) {
        size_t key = gen_key(stat->m_id, stat->m_tag);
        auto it = m_content.find(key);
        if (it != m_content.end())
            it->second = _sum_stats_as_copy(it->second, stat);
        else
            m_content.emplace(key, stat);
    }

    void StatsGrid::add_regular(double value, StatId id, Tag tag) {
        add(RegularStat::make(value, id, tag));
    }
    void StatsGrid::add_relative(const std::string& formula, StatId id, Tag tag) {
        add(RelativeStat::make(formula, id, tag));
    }

    void StatsGrid::add(const StatsGrid& stat) {
        for (const auto& v : stat.m_content | std::views::values)
            add(v);
    }

    void StatsGrid::_copy_from(const StatsGrid& another) {
        m_content.clear();
        for (const auto& [k, v] : another.m_content)
            m_content.emplace(k, v->copy_as_ptr());
    }

    // TODO: make it smaller and split between stats
    StatPtr StatsGrid::_sum_stats_as_copy(const StatPtr& l, const StatPtr& r) {
        StatPtr result;

        if (l->_type == 1 && r->_type == 1) {
            result = RegularStat::make(l->m_base + r->m_base, l->m_id, l->m_tag);
        } else if (l->_type == 1 && r->_type == 2) {
            auto lr = std::dynamic_pointer_cast<RelativeStat>(l);

            result = RelativeStat::make(
                l->m_base + r->m_base,
                lr->formulas(),
                l->m_id,
                l->m_tag
            );
        } else if (l->_type == 2 && r->_type == 1) {
            auto rr = std::dynamic_pointer_cast<RelativeStat>(r);

            result = RelativeStat::make(
                l->m_base + r->m_base,
                rr->formulas(),
                l->m_id,
                l->m_tag
            );
        } else if (l->_type == 2 && r->_type == 2) {
            auto lr = std::dynamic_pointer_cast<RelativeStat>(l);
            auto rr = std::dynamic_pointer_cast<RelativeStat>(r);

            RelativeStat::formulas_t formulas;
            const auto& lf = lr->formulas();
            const auto& rf = rr->formulas();

            { // conditions
                auto lt = lf.find('c');
                auto rt = rf.find('c');

                if (lt != lf.end() && rt == rf.end()) {
                    formulas['c'] = lt->second;
                } else if (lt == lf.end() && rt != rf.end()) {
                    formulas['c'] = rt->second;
                } else if (lt != lf.end() && rt != rf.end()) {
                    formulas['c'] = sum_rpns(lt->second, rt->second,
                        { .type = lib::rpn_parser::TokenType::And });
                }
            }

            { // functions
                formulas['f'] = sum_rpns(lf.at('f'), rf.at('f'),
                    { .type = lib::rpn_parser::TokenType::Plus });
            }

            { // max
                auto lt = lf.find('m');
                auto rt = rf.find('m');

                if (lt != lf.end() && rt == rf.end()) {
                    formulas['m'] = lt->second;
                } else if (lt == lf.end() && rt != rf.end()) {
                    formulas['c'] = rt->second;
                } else if (lt != lf.end() && rt != rf.end()) {
                    formulas['c'] = optimized_rpn_t {
                        lookup_token_t {
                            .type = lib::rpn_parser::Number,
                            .value = std::max(std::get<double>(lt->second[0].value),
                                std::get<double>(rt->second[0].value))
                        }
                    };
                }
            }

            result = RelativeStat::make(
                l->m_base + r->m_base,
                std::move(formulas),
                l->id(),
                l->tag()
            );
        } else
            throw RUNTIME_ERROR("wrong sum variant");

        return result;
    }
}
