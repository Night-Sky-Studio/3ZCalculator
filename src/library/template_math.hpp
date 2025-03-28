#pragma once

//library
#include "library/format.hpp"

namespace lib {
    inline double switch_math_op(double lhs, double rhs, char code) {
        switch (code) {
        case '+':
            return lhs + rhs;

        case '-':
            return lhs - rhs;

        case '*':
            return lhs * rhs;

        case '/':
            return lhs / rhs;

        case '%':
            return std::fmod(lhs, rhs);

        case '=':
            return std::fabs(lhs - rhs) < DBL_EPSILON;

        case '<':
            return lhs < rhs;

        case 0x80: // <=
            return lhs <= rhs;

        case '>':
            return lhs > rhs;

        case 0x81: // >=
            return lhs >= rhs;

        case '&':
            return (bool) lhs && (bool) rhs;

        case '|':
            return (bool) lhs || (bool) rhs;

        default:
            throw RUNTIME_ERROR("this math operator doesn't exist");
        }
    }
}
