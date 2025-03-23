#pragma once

//std
#include <vector>

//calculator
#include "calc/details.hpp"

#ifdef DEBUG_STATUS
#include "tabulate/table.hpp"
#endif

namespace calc {
    class Calculator {
    public:
        using result_t = std::tuple<double, std::vector<double>>;
        using detailed_result_t = std::tuple<double, std::vector<std::tuple<double, zzz::Tag, std::string>>>;

        static const enemy_t enemy;

        static result_t eval(const request_t& request);
        static detailed_result_t eval_detailed(const request_t& request);

#ifdef DEBUG_STATUS
        static tabulate::Table debug_stats(const request_t& request);
        static tabulate::Table debug_damage(const request_t& request, const result_t& damage);
#endif
    };
}
