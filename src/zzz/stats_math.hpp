#pragma once

//std
#include <cmath>

//zzz
#include "zzz/stats.hpp"

inline zzz::stat operator+(const zzz::stat& lhs, const zzz::stat& rhs) {
    return zzz::stat { .value = lhs.value + rhs.value, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator-(const zzz::stat& lhs, const zzz::stat& rhs) {
    return zzz::stat { .value = lhs.value - rhs.value, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator*(const zzz::stat& lhs, const zzz::stat& rhs) {
    return zzz::stat { .value = lhs.value * rhs.value, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator/(const zzz::stat& lhs, const zzz::stat& rhs) {
    return zzz::stat { .value = lhs.value / rhs.value, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator%(const zzz::stat& lhs, const zzz::stat& rhs) {
    return zzz::stat { .value = std::fmod(lhs.value, rhs.value), .type = lhs.type, .tag = lhs.tag };
}

inline zzz::stat operator+(const zzz::stat& lhs, double rhs) {
    return zzz::stat { .value = lhs.value + rhs, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator-(const zzz::stat& lhs, double rhs) {
    return zzz::stat { .value = lhs.value - rhs, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator*(const zzz::stat& lhs, double rhs) {
    return zzz::stat { .value = lhs.value * rhs, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator/(const zzz::stat& lhs, double rhs) {
    return zzz::stat { .value = lhs.value / rhs, .type = lhs.type, .tag = lhs.tag };
}
inline zzz::stat operator%(const zzz::stat& lhs, double rhs) {
    return zzz::stat { .value = std::fmod(lhs.value, rhs), .type = lhs.type, .tag = lhs.tag };
}
