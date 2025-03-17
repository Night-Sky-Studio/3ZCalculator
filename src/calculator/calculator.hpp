#pragma once

//std
#include <vector>

//calculator
#include "calculator/details.hpp"

//zzz
#include "zzz/details.hpp"

namespace calculator {
    class Calculator {
    public:
        using result_t = std::tuple<double, std::vector<double>>;

        static result_t eval(const eval_data_composed& composed, const eval_data_details& details);
    };
}
