#pragma once

//std
#include <string>
#include <vector>

//extern
#include "export_data.hpp"

//zzz
#include "zzz/combat.hpp"
#include "zzz/details.hpp"

namespace backend {
    struct enemy_details {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    class Calculator {
    public:
        static constexpr size_t level = 60;
        static constexpr double buff_level_mult = 1.0 + (level - 1.0) / 59.0;
        static constexpr double level_coefficient = 794.0;

        struct eval_data {
            const zzz::AgentDetails& agent;
            const zzz::WengineDetails& wengine;
            std::array<zzz::Ddp*, 6> drive_disks;
            std::vector<std::tuple<std::string, uint64_t>> rotation;
            const enemy_details& enemy;
        };

        static std::tuple<double, std::vector<double>> eval(const eval_data& data);

    private:
        static zzz::StatsGrid _precalc_stats(const eval_data& data);

        static double _calc_total_atk(const zzz::StatsGrid& stats, zzz::Tag tag);

        static double _calc_def_mult(const enemy_details& enemy, const zzz::StatsGrid& stats, zzz::Tag tag);
        static double _calc_dmg_taken_mult(const enemy_details& enemy, const zzz::StatsGrid& stats, zzz::Tag tag);
        static double _calc_res_mult(const enemy_details& enemy, const zzz::StatsGrid& stats, zzz::Element element,
            zzz::Tag tag);
        static double _calc_stun_mult(const enemy_details& enemy, const zzz::StatsGrid& stats);

        static double _calc_regular_dmg(
            const zzz::SkillDetails& skill,
            size_t index,
            zzz::StatsGrid stats,
            const enemy_details& enemy);
        static double _calc_anomaly_dmg(
            const zzz::AnomalyDetails& anomaly,
            zzz::Element element,
            zzz::StatsGrid stats,
            const enemy_details& enemy
        );
    };
}
