#pragma once

//std
#include <vector>

//backend
#include "calculator/details.hpp"
#include "calculator/object_manager.hpp"

//zzz
#include "zzz/details.hpp"

namespace calculator {
    class Calculator {
    public:
        static std::tuple<double, std::vector<double>> eval(ObjectManager& manager, const eval_data_details& details);
    };
}
