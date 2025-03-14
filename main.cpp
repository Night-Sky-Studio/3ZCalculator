//std
#include <array>
#include <memory>
#include <string>

//toml11
#include "toml.hpp"

//frozen
#include "frozen/string.h"

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/combat/drive_disc_piece.hpp"

//backend
#include "backend/converters.hpp"
#include "backend/calculator.hpp"
#include "backend/export_data.hpp"

int main() {
    backend::ToEvalDataConverter::init();

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = backend::global::to_eval_data.from(toml);
    auto result = backend::Calculator::eval(input);

    return 0;
}
