//std
#include <iostream>

//toml11
#include "toml.hpp"

//library
#include "library/funcs.hpp"

//zzz
#include "backend/backend.hpp"
#include "zzz/details.hpp"

//calculator
#include "calculator/calculator.hpp"
#include "calculator/converters.hpp"

//backend
#include "backend/backend.hpp"

int main() {
    backend::Backend server;
    auto& manager = server.manager();

    server.init();

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data_details.from(toml);
    auto result = calculator::Calculator::eval(input);

    server.run();

    return 0;
}
