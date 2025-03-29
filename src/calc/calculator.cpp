#include "calc/calculator.hpp"

//std
#include <map>
#include <ranges>

//lib
#include "library/format.hpp"

//zzz
#include "zzz/stats/grid.hpp"
#include "zzz/stats/relative.hpp"

using namespace zzz;

namespace calc::details {
    constexpr size_t level = 60;
    constexpr double buff_level_mult = 1.0 + (level - 1.0) / 59.0;
    constexpr double level_coefficient = 794.0;

    double calc_def_mult(const enemy_t& enemy, const StatsGrid& stats, Tag tag) {
        double effective_def = enemy.defense
            * (1 - stats.get_summed_value({ StatId::DefPenRatio, tag }))
            - stats.get_summed_value({ StatId::DefPenFlat, tag });
        return level_coefficient / (std::max(effective_def, 0.0) + level_coefficient);
    }
    double calc_dmg_taken_mult(const enemy_t& enemy, const StatsGrid& stats, Tag tag) {
        return 1.0
            - enemy.dmg_reduction
            + stats.get_summed_value({ StatId::Vulnerability, tag });
    }
    double calc_res_mult(const enemy_t& enemy, const StatsGrid& stats, Element element, Tag tag) {
        return 1.0 - enemy.res[element]
            + stats.get_summed_value({ StatId::ResPen, tag })
            + stats.get_summed_value({ StatId::ResPen + element, tag });
    }
    // TODO
    double calc_stun_mult(const enemy_t& enemy, const StatsGrid& stats) {
        return enemy.is_stunned ? enemy.stun_mult : 1.0;
    }

    double calc_regular_dmg(
        const SkillDetails& skill,
        size_t index,
        StatsGrid stats,
        const enemy_t& enemy) {
        const auto& scale = skill.scales()[index];

        stats.add(skill.buffs());
        stats.add(StatsGrid::make_defined_relative_stat(StatId::AtkTotal, Tag::Universal));

        double base_dmg = scale.motion_value / 100 * stats.get_value({ StatId::AtkTotal, Tag::Universal });
        double crit_mult = 1.0
            + std::min(stats.get_summed_value({ StatId::CritRate, skill.tag() }), 100.0)
            * stats.get_summed_value({ StatId::CritDmg, skill.tag() });
        double dmg_ratio_mult = 1.0
            + stats.get_summed_value({ StatId::DmgRatio, skill.tag() })
            + stats.get_summed_value({ StatId::DmgRatio + scale.element, skill.tag() });

        double dmg_taken_mult = calc_dmg_taken_mult(enemy, stats, skill.tag());
        double def_mult = calc_def_mult(enemy, stats, skill.tag());
        double res_mult = calc_res_mult(enemy, stats, scale.element, skill.tag());
        double stun_mult = 1.0 + calc_stun_mult(enemy, stats);

        return base_dmg
            * crit_mult
            * dmg_ratio_mult
            * dmg_taken_mult
            * def_mult
            * res_mult
            * stun_mult;
    }
    double calc_anomaly_dmg(
        const AnomalyDetails& anomaly,
        Element element,
        StatsGrid stats,
        const enemy_t& enemy) {
        stats.add(anomaly.buffs());
        stats.add(StatsGrid::make_defined_relative_stat(StatId::AtkTotal, Tag::Universal));

        double base_dmg = anomaly.scale() / 100 * stats.get_value({ StatId::AtkTotal, Tag::Universal });
        double crit_mult = 1.0 + (anomaly.can_crit()
            ? std::min(stats.get_value({ StatId::CritRate, Tag::Anomaly }), 100.0)
            * stats.get_value({ StatId::CritDmg, Tag::Anomaly })
            : 0.0);
        double dmg_ratio_mult = 1.0
            + stats.get_value({ StatId::DmgRatio, Tag::Universal })
            + stats.get_value({ StatId::DmgRatio + anomaly.element(), Tag::Universal });
        double anomaly_ratio_mult = 1.0
            + stats.get_value({ StatId::DmgRatio, Tag::Anomaly })
            + stats.get_value({ StatId::DmgRatio + anomaly.element(), Tag::Universal });

        double dmg_taken_mult = calc_dmg_taken_mult(enemy, stats, Tag::Anomaly);
        double def_mult = calc_def_mult(enemy, stats, Tag::Anomaly);
        double res_mult = calc_res_mult(enemy, stats, anomaly.element(), Tag::Anomaly);
        double stun_mult = 1.0 + calc_stun_mult(enemy, stats);

        return base_dmg
            * crit_mult
            * dmg_ratio_mult
            * anomaly_ratio_mult
            * dmg_taken_mult
            * def_mult
            * res_mult
            * stun_mult;
    }

    StatsGrid calc_stats(const request_t& request) {
        StatsGrid result;

        result.add(request.agent->details().stats());
        result.add(request.wengine->details().stats());

        for (size_t i = 0; i < 6; i++)
            result.add(request.ddps[i].stats());

        for (const auto& [count, value] : request.dds_by_count) {
            const auto& dds = value->details();

            if (count == 2)
                result.add(dds.pc2());
            else if (count == 4)
                result.add(dds.pc4());
        }

        return result;
    }
}

#ifdef DEBUG_STATUS

//std
#include <fstream>

namespace calc {
    tabulate::Table Calculator::debug_stats(const request_t& request) {
        StatsGrid summed_stats, agent_stats, wengine_stats, ddp_stats, dds_stats;

        agent_stats.add(request.agent->details().stats());
        wengine_stats.add(request.wengine->details().stats());

        for (size_t i = 0; i < 6; i++)
            ddp_stats.add(request.ddps[i].stats());

        for (const auto& [count, value] : request.dds_by_count) {
            const auto& dds = value->details();

            if (count == 2)
                dds_stats.add(dds.pc2());
            else if (count == 4)
                dds_stats.add(dds.pc4());
        }

        summed_stats.add(agent_stats);
        summed_stats.add(wengine_stats);
        summed_stats.add(ddp_stats);
        summed_stats.add(dds_stats);

        tabulate::Table stats_log;

        stats_log.add_row({ "agent", "wengine", "ddp", "dds", "total" });
        /*stats_log.add_row({
            agent_stats.get_debug_table(),
            wengine_stats.get_debug_table(),
            ddp_stats.get_debug_table(),
            dds_stats.get_debug_table(),
            summed_stats.get_debug_table()
        });*/

        std::fstream debug_file("stats.log", std::ios::out);
        stats_log.print(debug_file);

        return stats_log;
    }
    tabulate::Table Calculator::debug_damage(const request_t& request, const result_t& damage) {
        const auto& [total_dmg, dmg_per_ability] = damage;

        tabulate::Table dmg_log;
        size_t rounded_total_dmg = (size_t) total_dmg;

        dmg_log.add_row({ "ability", "dmg" });
        dmg_log.add_row({ "total", std::to_string(rounded_total_dmg) });

        const auto& rotation = request.rotation.ptr->details();
        for (size_t i = 0; i < rotation.size(); i++) {
            size_t rounded_dmg = (size_t) dmg_per_ability[i++];
            dmg_log.add_row({
                rotation[i].command + ' ' + std::to_string(rotation[i].index),
                lib::format("{}", rounded_dmg)
            });
        }

        std::fstream file("dmg.log", std::ios::out);
        dmg_log.print(file);

        return dmg_log;
    }
}
#endif

namespace calc {
    const enemy_t Calculator::enemy = {
        .dmg_reduction = 0.2,
        .defense = 953,
        .stun_mult = 1.5,
        .res = { 0.2, 0.2, 0.2, 0.2, 0.2 },
        .is_stunned = false
    };

    Calculator::result_t Calculator::eval(const request_t& request) {
        const auto& agent = request.agent->details();
        const auto& rotation = request.rotation->details();

        double total_dmg = 0.0;
        std::vector<double> dmg_per_ability;
        auto stats = details::calc_stats(request);

        dmg_per_ability.reserve(rotation.size());
        for (size_t i = 0; i < rotation.size(); i++) {
            const auto& cell = rotation[i];
            const auto& ability = agent.ability(cell.command);
            double dmg;

            if (std::holds_alternative<SkillDetails>(ability)) {
                const auto& skill = std::get<SkillDetails>(ability);
                dmg = details::calc_regular_dmg(skill, cell.index - 1, stats, enemy);
            } else if (std::holds_alternative<AnomalyDetails>(ability)) {
                const auto& anomaly = std::get<AnomalyDetails>(ability);
                dmg = details::calc_anomaly_dmg(anomaly, agent.element(), stats, enemy);
            } else
                throw RUNTIME_ERROR("ability is neither skill nor anomaly");

            total_dmg += dmg;
            dmg_per_ability.emplace_back(dmg);
        }

        return { total_dmg, dmg_per_ability };
    }
    Calculator::detailed_result_t Calculator::eval_detailed(const request_t& request) {
        const auto& agent = request.agent->details();
        const auto& rotation = request.rotation->details();

        double total_dmg = 0.0;
        std::vector<std::tuple<double, Tag, std::string>> info_per_ability;
        auto stats = details::calc_stats(request);

        info_per_ability.reserve(rotation.size());
        for (size_t i = 0; i < rotation.size(); i++) {
            auto cell = rotation[i];
            const auto& ability = agent.ability(cell.command);
            double dmg;
            Tag tag;

            if (std::holds_alternative<SkillDetails>(ability)) {
                const auto& skill = std::get<SkillDetails>(ability);
                dmg = details::calc_regular_dmg(skill, cell.index - 1, stats, enemy);
                tag = skill.tag();
                if (skill.max_index() > 1)
                    cell.command += ' ' + std::to_string(cell.index);
            } else if (std::holds_alternative<AnomalyDetails>(ability)) {
                const auto& anomaly = std::get<AnomalyDetails>(ability);
                dmg = details::calc_anomaly_dmg(anomaly, agent.element(), stats, enemy);
                tag = Tag::Anomaly;
            } else
                throw RUNTIME_ERROR("ability is neither skill nor anomaly");

            total_dmg += dmg;
            info_per_ability.emplace_back(dmg, tag, std::move(cell.command));
        }

        return { total_dmg, info_per_ability };
    }
}
