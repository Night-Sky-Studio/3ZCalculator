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
        static std::tuple<double, std::vector<double>> eval(ObjectManager& manager, const eval_data_details& details);
    };
}
