#include "backend/calculator.hpp"

//std
#include <map>
#include <ranges>

//zzz
#include "zzz/stats_math.hpp"

#ifdef DEBUG_STATUS
#include <fstream>
#include "library/funcs.hpp"
#include "tabulate/table.hpp"
#endif

using namespace zzz;

namespace backend {
    std::tuple<double, std::vector<double>> Calculator::eval(ObjectManager& manager, const eval_data_details& data) {
        double total_dmg = 0.0;
        std::vector<double> dmg_per_skill;
        StatsGrid stats;

        AgentDetailsPtr agent;
        WengineDetailsPtr wengine;
        std::multimap<size_t, DdsDetailsPtr> dds;
        rotation_details_ptr rotation;

        {
            auto agent_future = manager.get(data.agent_id);
            auto wengine_future = manager.get(data.wengine_id);
            auto rotation_future = manager.get(data.rotation_id);

            std::map<uint64_t, size_t> dds_count;
            for (const auto& it : data.drive_disks) {
                if (auto jt = dds_count.find(it.disc_id()); jt != dds_count.end())
                    jt->second++;
                else
                    dds_count[it.disc_id()] = 1;
            }

            std::list<std::future<any_ptr>> dds_futures;
            for (const auto& [id, count] : dds_count) {
                if (count >= 2)
                    dds_futures.emplace_back(manager.get(id));
            }

            agent = std::static_pointer_cast<AgentDetails>(agent_future.get());
            wengine = std::static_pointer_cast<WengineDetails>(wengine_future.get());
            rotation = std::static_pointer_cast<rotation_details>(rotation_future.get());

            for (auto& future : dds_futures) {
                auto ptr = std::static_pointer_cast<DdsDetails>(future.get());
                auto id = dds_count.at(ptr->id());

                if (id >= 2)
                    dds.emplace(2, ptr);
                if (id >= 4)
                    dds.emplace(4, ptr);
            }
        }

        {
            StatsGrid agent_stats, wengine_stats, ddp_stats, dds_stats;

            agent_stats.add(agent->stats());

            wengine_stats.add(wengine->main_stat());
            wengine_stats.add(wengine->sub_stat());
            wengine_stats.add(wengine->passive_stats());

            for (const auto& it : data.drive_disks) {
                ddp_stats.add(it.main_stat());
                for (size_t i = 0; i < 4; i++)
                    ddp_stats.add(it.sub_stat(i));
            }

            for (const auto& [count, set] : dds) {
                if (count == 2)
                    dds_stats.add(set->p2());
                if (count == 4)
                    dds_stats.add(set->p4());
            }

            stats.add(agent_stats);
            stats.add(wengine_stats);
            stats.add(ddp_stats);
            stats.add(dds_stats);

#ifdef DEBUG_STATS
            {
                tabulate::Table stats_log;

                stats_log.add_row({ "agent", "wengine", "ddp", "dds", "total" });
                stats_log.add_row({
                    agent_stats.get_debug_table(),
                    wengine_stats.get_debug_table(),
                    ddp_stats.get_debug_table(),
                    dds_stats.get_debug_table(),
                    stats.get_debug_table()
                });

                std::fstream debug_file("stats.log", std::ios::out);
                stats_log.print(debug_file);
            }
#endif
        }

        dmg_per_skill.reserve(rotation->size());
        for (const auto& [ability_name, index] : *rotation) {
            double dmg = AgentDetails::is_skill_or_anomaly(*agent, ability_name) == 1
                ? _calc_regular_dmg(agent->skill(ability_name), index - 1, stats, data.enemy)
                : _calc_anomaly_dmg(agent->anomaly(ability_name), agent->element(), stats, data.enemy);

            total_dmg += dmg;
            dmg_per_skill.emplace_back(dmg);
        }

#ifdef DEBUG_STATUS
        {
            tabulate::Table dmg_log;
            size_t rounded_total_dmg = total_dmg;

            dmg_log.add_row({ "ability", "dmg" });
            dmg_log.add_row({ "total", lib::format("{}", rounded_total_dmg) });

            size_t i = 0;
            for (const auto& cell : *rotation) {
                size_t rounded_dmg = dmg_per_skill[i++];
                dmg_log.add_row({ 
                    cell.command + ' ' + std::to_string(cell.index),
                    lib::format("{}", rounded_dmg)
                });
            }

            std::fstream file("dmg.log", std::ios::out);
            dmg_log.print(file);
        }
#endif

        return { total_dmg, dmg_per_skill };
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

        double base_dmg = anomaly.scale() / 100 * stats.get(StatType::AtkTotal);
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
