//backend
#include "backend/backend.hpp"

int main() {
    backend::Backend server;

    server.init();
    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data_details.from(toml);
    auto result = calculator::Calculator::eval(input);

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data_details.from(toml);
    auto result = calculator::Calculator::eval(input);

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data_details.from(toml);
    auto result = calculator::Calculator::eval(input);

    server.run();

    return 0;
}
