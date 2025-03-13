//std
#include <array>
#include <memory>
#include <string>

//toml11
#include "toml.hpp"

//frozen
#include "frozen/string.h"

//zzz
#include "zzz/combat/drive_disc_piece.hpp"

//backend
#include "backend/calculator.hpp"
#include "backend/export_data.hpp"

using namespace zzz;

toml::value load_by_id(const std::string& folder, uint64_t id) {
    std::fstream file(folder + '/' + std::to_string(id) + ".toml");
    return toml::parse(file);
}

int main() {
    auto agent = load_by_id("agent", 1091);

    return 0;
}
