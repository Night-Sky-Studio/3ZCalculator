#pragma once

//std
#include <functional>

//frozen
#include "frozen/map.h"

#define DO_DOUBLE_MATH_OP(CH, OP) { CH, lib::do_math_op<double, OP<double>> }

namespace lib {
    template<typename T, typename TFunctor>
    T do_math_op(T l, T r) {
        return TFunctor()(l, r);
    }

    // based on rpn_tokens
    constexpr frozen::map<char, std::function<double(double, double)>, 12> math_ops = {
        DO_DOUBLE_MATH_OP('+', std::plus),
        DO_DOUBLE_MATH_OP('-', std::minus),
        DO_DOUBLE_MATH_OP('*', std::multiplies),
        DO_DOUBLE_MATH_OP('/', std::divides),
        DO_DOUBLE_MATH_OP('=', std::equal_to),
        DO_DOUBLE_MATH_OP('<', std::less),
        DO_DOUBLE_MATH_OP(0x80, std::less_equal),
        DO_DOUBLE_MATH_OP('>', std::greater),
        DO_DOUBLE_MATH_OP(0x81, std::greater_equal),
        DO_DOUBLE_MATH_OP('&', std::logical_and),
        DO_DOUBLE_MATH_OP('|', std::logical_or)
    };
}
