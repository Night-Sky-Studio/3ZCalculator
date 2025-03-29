#pragma once

//std
#include <chrono>
#include <functional>
#include <string>

//crow
#include "crow/logging.h"

namespace backend::details {
    // timers

    template<typename TResult, typename... TArgs>
    TResult wrap_to_check_execution_time(
        std::string_view name,
        std::function<TResult()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func();
        std::chrono::duration<double, std::milli> time =
            std::chrono::high_resolution_clock::now() - start;

        CROW_LOG_INFO << lib::format("{} was dispatched in {} ms", name, time.count());

        return result;
    }
}
