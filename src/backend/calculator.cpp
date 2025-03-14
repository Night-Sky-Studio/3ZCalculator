#include "backend/calculator.hpp"

//zzz
#include "zzz/stats_math.hpp"

using namespace zzz;

namespace backend {
    std::tuple<double, std::vector<double>> Calculator::eval(const eval_data_details& data) {
        double total_dmg = 0.0;
        std::vector<double> dmg_per_skill;

        auto stats = _precalc_stats(data);

        dmg_per_skill.reserve(data.rotation.size());
        for (const auto& [ability_name, index] : data.rotation) {
            double dmg = AgentDetails::is_skill_or_anomaly(data.agent, ability_name) == 1
                ? _calc_regular_dmg(data.agent.skill(ability_name), index - 1, stats, data.enemy)
                : _calc_anomaly_dmg(data.agent.anomaly(ability_name), data.agent.element(), stats, data.enemy);

            total_dmg += dmg;
            dmg_per_skill.emplace_back(dmg);
        }

        return { total_dmg, dmg_per_skill };
    }

    StatsGrid Calculator::_precalc_stats(const eval_data_details& data) {
        StatsGrid result;

        result.add(data.agent.stats());
        result.add(data.wengine.main_stat());
        result.add(data.wengine.sub_stat());
        result.add(data.wengine.passive_stats());
        for (const auto& it : data.drive_disks) {
            result.add(it.main_stat());
            result.add(it.sub_stat(0));
            result.add(it.sub_stat(1));
            result.add(it.sub_stat(2));
            result.add(it.sub_stat(3));
        }

        return result;
    }

    // TODO: make part of StatsGrid
    double Calculator::_calc_total_atk(const StatsGrid& stats, Tag tag) {
        return stats.get(StatType::AtkBase) * (1 + stats.get_all(StatType::AtkRatio, tag))
            + stats.get(StatType::AtkFlat);
    }

    double Calculator::_calc_def_mult(const enemy_details& enemy, const StatsGrid& stats, Tag tag) {
        double effective_def = enemy.defense * (1 - stats.get_all(StatType::DefPenRatio, tag))
            - stats.get_all(StatType::DefPenFlat, tag);
        return level_coefficient / (std::max(effective_def, 0.0) + level_coefficient);
    }
    double Calculator::_calc_dmg_taken_mult(const enemy_details& enemy, const StatsGrid& stats, Tag tag) {
        return 1.0
            - enemy.dmg_reduction
            + stats.get_all(StatType::Vulnerability, tag);
    }
    double Calculator::_calc_res_mult(const enemy_details& enemy, const StatsGrid& stats, Element element, Tag tag) {
        return 1.0
            - enemy.res[(size_t) element]
            + stats.get_all(StatType::ResPen, tag)
            + stats.get_all(StatType::ResPen + element, tag);
    }
    // TODO
    double Calculator::_calc_stun_mult(const enemy_details& enemy, const StatsGrid& stats) {
        return enemy.is_stunned ? enemy.stun_mult : 1.0;
    }

    double Calculator::_calc_regular_dmg(
        const SkillDetails& skill,
        size_t index,
        StatsGrid stats,
        const enemy_details& enemy) {
        const auto& scale = skill.scales()[index];

        stats.add(skill.buffs());
        stats.at(StatType::AtkTotal).value = _calc_total_atk(stats, skill.tag());

        double base_dmg = scale.motion_value / 100 * stats.at(StatType::AtkTotal);
        double crit_mult = 1.0
            + stats.get_all(StatType::CritRate, skill.tag())
            * stats.get_all(StatType::CritDmg, skill.tag());
        double dmg_ratio_mult = 1.0
            + stats.get_all(StatType::DmgRatio, skill.tag())
            + stats.get_all(StatType::DmgRatio + scale.element, skill.tag());

        double dmg_taken_mult = _calc_dmg_taken_mult(enemy, stats, skill.tag());
        double def_mult = _calc_def_mult(enemy, stats, skill.tag());
        double res_mult = _calc_res_mult(enemy, stats, scale.element, skill.tag());
        double stun_mult = 1.0 + _calc_stun_mult(enemy, stats);

        return base_dmg * crit_mult * dmg_ratio_mult * dmg_taken_mult * def_mult * res_mult * stun_mult;
    }

    double Calculator::_calc_anomaly_dmg(
        const AnomalyDetails& anomaly,
        Element element,
        StatsGrid stats,
        const enemy_details& enemy) {
        stats.add(anomaly.buffs());
        stats.at(StatType::AtkTotal).value = _calc_total_atk(stats, Tag::Anomaly);

        double base_dmg = anomaly.scale() * stats.get(StatType::AtkTotal);
        double crit_mult = 1.0 + (anomaly.can_crit()
            ? stats.get_only(StatType::CritRate, Tag::Anomaly) * stats.get_only(StatType::CritDmg, Tag::Anomaly)
            : 0.0);
        double dmg_ratio_mult = 1.0 + stats.get(StatType::DmgRatio) + stats.get(StatType::DmgRatio + element);
        double anomaly_ratio_mult = 1.0
            + stats.get_only(StatType::DmgRatio, Tag::Anomaly)
            + stats.get_only(StatType::DmgRatio + element, Tag::Anomaly);
        double ap_bonus_mult = stats.get(StatType::Ap) / 100.0;

        double dmg_taken_mult = _calc_dmg_taken_mult(enemy, stats, Tag::Anomaly);
        double def_mult = _calc_def_mult(enemy, stats, Tag::Anomaly);
        double res_mult = _calc_res_mult(enemy, stats, element, Tag::Anomaly);
        double stun_mult = 1.0 + _calc_stun_mult(enemy, stats);

        return base_dmg * crit_mult * dmg_ratio_mult * anomaly_ratio_mult * ap_bonus_mult * buff_level_mult *
            dmg_taken_mult * def_mult * res_mult * stun_mult;
    }
}
