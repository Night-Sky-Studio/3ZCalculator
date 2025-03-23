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

        static const enemy_t enemy;

        static result_t eval(const Request& request);

#ifdef DEBUG_STATUS
        static tabulate::Table debug_stats(const Request& request);
        static tabulate::Table debug_damage(const Request& request, const result_t& damage);
#endif
    };
}
