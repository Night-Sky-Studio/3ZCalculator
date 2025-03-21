#include "calc/calculator.hpp"

//std
#include <map>
#include <ranges>

//zzz
#include "zzz/stats_math.hpp"

using namespace zzz;

namespace calc {
    constexpr size_t level = 60;
    constexpr double buff_level_mult = 1.0 + (level - 1.0) / 59.0;
    constexpr double level_coefficient = 794.0;

    // TODO: make part of StatsGrid
    double calc_total_atk(const StatsGrid& stats, Tag tag) {
        return stats.get(StatType::AtkBase) * (1 + stats.get_all(StatType::AtkRatio, tag))
            + stats.get(StatType::AtkFlat);
    }

    double calc_def_mult(const enemy_t& enemy, const StatsGrid& stats, Tag tag) {
        double effective_def = enemy.defense * (1 - stats.get_all(StatType::DefPenRatio, tag))
            - stats.get_all(StatType::DefPenFlat, tag);
        return level_coefficient / (std::max(effective_def, 0.0) + level_coefficient);
    }
    double calc_dmg_taken_mult(const enemy_t& enemy, const StatsGrid& stats, Tag tag) {
        return 1.0
            - enemy.dmg_reduction
            + stats.get_all(StatType::Vulnerability, tag);
    }
    double calc_res_mult(const enemy_t& enemy, const StatsGrid& stats, Element element, Tag tag) {
        return 1.0
            - enemy.res[(size_t) element]
            + stats.get_all(StatType::ResPen, tag)
            + stats.get_all(StatType::ResPen + element, tag);
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
        stats.at(StatType::AtkTotal).value = calc_total_atk(stats, skill.tag());

        double base_dmg = scale.motion_value / 100 * stats.at(StatType::AtkTotal);
        double crit_mult = 1.0
            + stats.get_all(StatType::CritRate, skill.tag())
            * stats.get_all(StatType::CritDmg, skill.tag());
        double dmg_ratio_mult = 1.0
            + stats.get_all(StatType::DmgRatio, skill.tag())
            + stats.get_all(StatType::DmgRatio + scale.element, skill.tag());

        double dmg_taken_mult = calc_dmg_taken_mult(enemy, stats, skill.tag());
        double def_mult = calc_def_mult(enemy, stats, skill.tag());
        double res_mult = calc_res_mult(enemy, stats, scale.element, skill.tag());
        double stun_mult = 1.0 + calc_stun_mult(enemy, stats);

        return base_dmg * crit_mult * dmg_ratio_mult * dmg_taken_mult * def_mult * res_mult * stun_mult;
    }
    double calc_anomaly_dmg(
        const AnomalyDetails& anomaly,
        Element element,
        StatsGrid stats,
        const enemy_t& enemy) {
        stats.add(anomaly.buffs());
        stats.at(StatType::AtkTotal).value = calc_total_atk(stats, Tag::Anomaly);

        double base_dmg = anomaly.scale() / 100 * stats.get(StatType::AtkTotal);
        double crit_mult = 1.0 + (anomaly.can_crit()
            ? stats.get_only(StatType::CritRate, Tag::Anomaly) * stats.get_only(StatType::CritDmg, Tag::Anomaly)
            : 0.0);
        double dmg_ratio_mult = 1.0 + stats.get(StatType::DmgRatio) + stats.get(StatType::DmgRatio + element);
        double anomaly_ratio_mult = 1.0
            + stats.get_only(StatType::DmgRatio, Tag::Anomaly)
            + stats.get_only(StatType::DmgRatio + element, Tag::Anomaly);
        double ap_bonus_mult = stats.get(StatType::Ap) / 100.0;

        double dmg_taken_mult = calc_dmg_taken_mult(enemy, stats, Tag::Anomaly);
        double def_mult = calc_def_mult(enemy, stats, Tag::Anomaly);
        double res_mult = calc_res_mult(enemy, stats, element, Tag::Anomaly);
        double stun_mult = 1.0 + calc_stun_mult(enemy, stats);

        return base_dmg * crit_mult * dmg_ratio_mult * anomaly_ratio_mult * ap_bonus_mult * buff_level_mult *
            dmg_taken_mult * def_mult * res_mult * stun_mult;
    }
}

namespace calc {
    StatsGrid calc_stats(const request_t& request) {
        StatsGrid result;

        result.add(request.agent.ptr->stats());

        result.add(request.wengine.ptr->main_stat());
        result.add(request.wengine.ptr->sub_stat());
        result.add(request.wengine.ptr->passive_stats());

        for (const auto& it : request.ddps) {
            result.add(it.main_stat());
            for (size_t i = 0; i < 4; i++)
                result.add(it.sub_stat(i));
        }

        for (const auto& [count, set] : request.dds.by_count) {
            if (count == 2)
                result.add((*set)->p2());
            else if (count == 4)
                result.add((*set)->p4());
        }

        return result;
    }

    std::tuple<double, std::vector<double>> request_dmg(const request_t& request) {
        double total_dmg = 0.0;
        std::vector<double> dmg_per_ability;
        auto stats = calc_stats(request);

        dmg_per_ability.reserve(request.rotation.ptr->size());
        for (const auto& [ability_name, index] : *request.rotation.ptr) {
            size_t id = AgentDetails::is_skill_or_anomaly(*request.agent.ptr, ability_name);
            double dmg;

            switch (id) {
            case 1:
                dmg = calc_regular_dmg(
                    request.agent.ptr->skill(ability_name),
                    index - 1,
                    stats,
                    Calculator::enemy
                );
                break;
            case 2:
                dmg = calc_anomaly_dmg(
                    request.agent.ptr->anomaly(ability_name),
                    request.agent.ptr->element(),
                    stats,
                    Calculator::enemy
                );
                break;
            default:
                throw std::runtime_error("ability is neither skill nor anomaly");
            }

            total_dmg += dmg;
            dmg_per_ability.emplace_back(dmg);
        }

        return { total_dmg, dmg_per_ability };
    }
}

#ifdef DEBUG_STATUS

//std
#include <fstream>

//library
#include "library/format.hpp"

namespace calc {
    tabulate::Table Calculator::debug_stats(const request_t& request) {
        StatsGrid summed_stats, agent_stats, wengine_stats, ddp_stats, dds_stats;

        agent_stats.add(request.agent.ptr->stats());

        wengine_stats.add(request.wengine.ptr->main_stat());
        wengine_stats.add(request.wengine.ptr->sub_stat());
        wengine_stats.add(request.wengine.ptr->passive_stats());

        for (const auto& it : request.ddps) {
            ddp_stats.add(it.main_stat());
            for (size_t i = 0; i < 4; i++)
                ddp_stats.add(it.sub_stat(i));
        }

        for (const auto& [count, set] : request.dds.by_count) {
            if (count == 2)
                dds_stats.add((*set)->p2());
            if (count == 4)
                dds_stats.add((*set)->p4());
        }

        summed_stats.add(agent_stats);
        summed_stats.add(wengine_stats);
        summed_stats.add(ddp_stats);
        summed_stats.add(dds_stats);

        tabulate::Table stats_log;

        stats_log.add_row({ "agent", "wengine", "ddp", "dds", "total" });
        stats_log.add_row({
            agent_stats.get_debug_table(),
            wengine_stats.get_debug_table(),
            ddp_stats.get_debug_table(),
            dds_stats.get_debug_table(),
            summed_stats.get_debug_table()
        });

        std::fstream debug_file("stats.log", std::ios::out);
        stats_log.print(debug_file);

        return stats_log;
    }
    tabulate::Table Calculator::debug_damage(const request_t& request, const result_t& damage) {
        const auto& [total_dmg, dmg_per_ability] = damage;

        tabulate::Table dmg_log;
        size_t rounded_total_dmg = total_dmg;

        dmg_log.add_row({ "ability", "dmg" });
        dmg_log.add_row({ "total", std::to_string(rounded_total_dmg) });

        size_t i = 0;
        for (const auto& cell : *request.rotation.ptr) {
            size_t rounded_dmg = dmg_per_ability[i++];
            dmg_log.add_row({
                cell.command + ' ' + std::to_string(cell.index),
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
        auto result = request_dmg(request);
        return result;
    }
}
