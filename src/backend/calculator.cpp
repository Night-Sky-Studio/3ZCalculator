#include "backend/calculator.hpp"

//std
#include <map>
#include <ranges>

//zzz
#include "zzz/stats_math.hpp"

using namespace zzz;

namespace backend {
    struct eval_data_composed {
        AgentDetailsPtr agent;
        WengineDetailsPtr wengine;
        std::multimap<size_t, DdsDetailsPtr> dds;
        rotation_details_ptr rotation;
    };
}

namespace backend::calculator_eval {
    constexpr size_t level = 60;
    constexpr double buff_level_mult = 1.0 + (level - 1.0) / 59.0;
    constexpr double level_coefficient = 794.0;

    // TODO: make part of StatsGrid
    double calc_total_atk(const StatsGrid& stats, Tag tag) {
        return stats.get(StatType::AtkBase) * (1 + stats.get_all(StatType::AtkRatio, tag))
            + stats.get(StatType::AtkFlat);
    }

    double calc_def_mult(const enemy_details& enemy, const StatsGrid& stats, Tag tag) {
        double effective_def = enemy.defense * (1 - stats.get_all(StatType::DefPenRatio, tag))
            - stats.get_all(StatType::DefPenFlat, tag);
        return level_coefficient / (std::max(effective_def, 0.0) + level_coefficient);
    }
    double calc_dmg_taken_mult(const enemy_details& enemy, const StatsGrid& stats, Tag tag) {
        return 1.0
            - enemy.dmg_reduction
            + stats.get_all(StatType::Vulnerability, tag);
    }
    double calc_res_mult(const enemy_details& enemy, const StatsGrid& stats, Element element, Tag tag) {
        return 1.0
            - enemy.res[(size_t) element]
            + stats.get_all(StatType::ResPen, tag)
            + stats.get_all(StatType::ResPen + element, tag);
    }
    // TODO
    double calc_stun_mult(const enemy_details& enemy, const StatsGrid& stats) {
        return enemy.is_stunned ? enemy.stun_mult : 1.0;
    }

    double calc_regular_dmg(
        const SkillDetails& skill,
        size_t index,
        StatsGrid stats,
        const enemy_details& enemy) {
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
        const enemy_details& enemy) {
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

#ifdef DEBUG_STATUS

//std
#include <fstream>

//tabulate
#include "tabulate/table.hpp"

//library
#include "library/funcs.hpp"

namespace backend::calculator_requests {
    eval_data_composed request_data(ObjectManager& manager, const eval_data_details& details) {
        eval_data_composed result;

        auto agent_future = manager.get(details.agent_id);
        auto wengine_future = manager.get(details.wengine_id);
        auto rotation_future = manager.get(details.rotation_id);

        std::map<uint64_t, size_t> dds_count;
        for (const auto& it : details.drive_discs) {
            if (auto jt = dds_count.find(it.disc_id()); jt != dds_count.end())
                jt->second++;
            else
                dds_count[it.disc_id()] = 1;
        }

        std::list<std::future<any_ptr>> dds_futures;
        for (const auto& [id, count] : dds_count) {
            if (count >= 2)
                dds_futures.emplace_back(manager.get("dds/" + std::to_string(id)));
        }

        try {
            result.agent = std::static_pointer_cast<AgentDetails>(agent_future.get());
            result.wengine = std::static_pointer_cast<WengineDetails>(wengine_future.get());
            result.rotation = std::static_pointer_cast<rotation_details>(rotation_future.get());

            for (auto& future : dds_futures) {
                auto ptr = std::static_pointer_cast<DdsDetails>(future.get());
                auto id = dds_count.at(ptr->id());

                if (id >= 2)
                    result.dds.emplace(2, ptr);
                if (id >= 4)
                    result.dds.emplace(4, ptr);
            }
        } catch (const std::runtime_error& e) {
            std::string message = lib::format("error: {}\n", e.what());
            std::cerr << message;
        }

        return result;
    }

    StatsGrid request_stats(const eval_data_composed& composed, const eval_data_details& details) {
        StatsGrid result_stats, agent_stats, wengine_stats, ddp_stats, dds_stats;

        agent_stats.add(composed.agent->stats());

        wengine_stats.add(composed.wengine->main_stat());
        wengine_stats.add(composed.wengine->sub_stat());
        wengine_stats.add(composed.wengine->passive_stats());

        for (const auto& it : details.drive_discs) {
            ddp_stats.add(it.main_stat());
            for (size_t i = 0; i < 4; i++)
                ddp_stats.add(it.sub_stat(i));
        }

        for (const auto& [count, set] : composed.dds) {
            if (count == 2)
                dds_stats.add(set->p2());
            if (count == 4)
                dds_stats.add(set->p4());
        }

        result_stats.add(agent_stats);
        result_stats.add(wengine_stats);
        result_stats.add(ddp_stats);
        result_stats.add(dds_stats);

        tabulate::Table stats_log;

        stats_log.add_row({ "agent", "wengine", "ddp", "dds", "total" });
        stats_log.add_row({
            agent_stats.get_debug_table(),
            wengine_stats.get_debug_table(),
            ddp_stats.get_debug_table(),
            dds_stats.get_debug_table(),
            result_stats.get_debug_table()
        });

        std::fstream debug_file("stats.log", std::ios::out);
        stats_log.print(debug_file);

        return result_stats;
    }

    std::tuple<double, std::vector<double>> request_dmg(
        const StatsGrid& stats,
        const eval_data_composed& composed,
        const eval_data_details& details) {
        double total_dmg = 0.0;
        std::vector<double> dmg_per_skill;

        dmg_per_skill.reserve(composed.rotation->size());
        for (const auto& [ability_name, index] : *composed.rotation) {
            size_t id = AgentDetails::is_skill_or_anomaly(*composed.agent, ability_name);
            double dmg;

            switch (id) {
            case 1:
                dmg = calculator_eval::calc_regular_dmg(
                    composed.agent->skill(ability_name),
                    index - 1,
                    stats,
                    details.enemy
                );
                break;
            case 2:
                dmg = calculator_eval::calc_anomaly_dmg(
                    composed.agent->anomaly(ability_name),
                    composed.agent->element(),
                    stats,
                    details.enemy
                );
                break;
            default:
                throw std::runtime_error("ability is neither skill nor anomaly");
            }

            total_dmg += dmg;
            dmg_per_skill.emplace_back(dmg);
        }

        tabulate::Table dmg_log;
        size_t rounded_total_dmg = total_dmg;

        dmg_log.add_row({ "ability", "dmg" });
        dmg_log.add_row({ "total", std::to_string(rounded_total_dmg) });

        size_t i = 0;
        for (const auto& cell : *composed.rotation) {
            size_t rounded_dmg = dmg_per_skill[i++];
            dmg_log.add_row({
                cell.command + ' ' + std::to_string(cell.index),
                lib::format("{}", rounded_dmg)
            });
        }

        std::fstream file("dmg.log", std::ios::out);
        dmg_log.print(file);

        return { total_dmg, dmg_per_skill };
    }
}
#else
namespace backend::calculator_requests {
    eval_data_composed request_data(ObjectManager& manager, const eval_data_details& details) {
        eval_data_composed result;

        auto agent_future = manager.get(details.agent_id);
        auto wengine_future = manager.get(details.wengine_id);
        auto rotation_future = manager.get(details.rotation_id);

        std::map<uint64_t, size_t> dds_count;
        for (const auto& it : details.drive_disks) {
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

        result.agent = std::static_pointer_cast<AgentDetails>(agent_future.get());
        result.wengine = std::static_pointer_cast<WengineDetails>(wengine_future.get());
        result.rotation = std::static_pointer_cast<rotation_details>(rotation_future.get());

        for (auto& future : dds_futures) {
            auto ptr = std::static_pointer_cast<DdsDetails>(future.get());
            auto id = dds_count.at(ptr->id());

            if (id >= 2)
                result.dds.emplace(2, ptr);
            if (id >= 4)
                result.dds.emplace(4, ptr);
        }

        return result;
    }

    StatsGrid request_stats(const eval_data_composed& composed, const eval_data_details& details) {
        StatsGrid result;

        result.add(composed.agent->stats());

        result.add(composed.wengine->main_stat());
        result.add(composed.wengine->sub_stat());
        result.add(composed.wengine->passive_stats());

        for (const auto& it : details.drive_disks) {
            result.add(it.main_stat());
            for (size_t i = 0; i < 4; i++)
                result.add(it.sub_stat(i));
        }

        for (const auto& [count, set] : composed.dds) {
            if (count == 2)
                result.add(set->p2());
            if (count == 4)
                result.add(set->p4());
        }

        return result;
    }

    std::tuple<double, std::vector<double>> request_dmg(
        const StatsGrid& stats,
        const eval_data_composed& composed,
        const eval_data_details& details) {
        double total_dmg = 0.0;
        std::vector<double> dmg_per_skill;

        dmg_per_skill.reserve(composed.rotation->size());
        for (const auto& [ability_name, index] : *composed.rotation) {
            size_t id = AgentDetails::is_skill_or_anomaly(*composed.agent, ability_name);
            double dmg;

            switch (id) {
            case 1:
                dmg = calculator_eval::calc_regular_dmg(
                    composed.agent->skill(ability_name),
                    index - 1,
                    stats,
                    details.enemy
                );
                break;
            case 2:
                dmg = calculator_eval::calc_anomaly_dmg(
                    composed.agent->anomaly(ability_name),
                    composed.agent->element(),
                    stats,
                    details.enemy
                );
                break;
            default:
                dmg = 0;
            }

            total_dmg += dmg;
            dmg_per_skill.emplace_back(dmg);
        }

        return { total_dmg, dmg_per_skill };
    }
}
#endif

namespace backend {
    std::tuple<double, std::vector<double>> Calculator::eval(ObjectManager& manager, const eval_data_details& details) {
        auto composed = calculator_requests::request_data(manager, details);
        auto stats = calculator_requests::request_stats(composed, details);
        auto result = calculator_requests::request_dmg(stats, composed, details);
        return result;
    }
}
