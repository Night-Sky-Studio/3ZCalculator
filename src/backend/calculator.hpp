#pragma once

//std
#include <vector>

//backend
#include "backend/details.hpp"
#include "backend/object_manager.hpp"

//zzz
#include "zzz/details.hpp"

namespace backend {
    class Calculator {
    public:
        static constexpr size_t level = 60;
        static constexpr double buff_level_mult = 1.0 + (level - 1.0) / 59.0;
        static constexpr double level_coefficient = 794.0;

        static std::tuple<double, std::vector<double>> eval(ObjectManager& manager, const eval_data_details& data);

    private:
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
