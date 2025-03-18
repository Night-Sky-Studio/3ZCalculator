cmake_minimum_required(VERSION 3.28)

project(
    3ZCalculator
    VERSION 0.1.0
    DESCRIPTION "Calculates damage of ZZZ character with rough approximation"
    HOMEPAGE_URL "https://github.com/Night-Sky-Studio/3ZCalculator"
)
set(CMAKE_CXX_STANDARD 23)

# FetchContent
include(FetchContent)

set(USE_BOOST_KARMA OFF)

# CPM.cmake

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

# FrozenLib
FetchContent_Declare(
    FrozenLib
    GIT_REPOSITORY https://github.com/serge-sans-paille/frozen
    GIT_TAG        master
)
FetchContent_MakeAvailable(FrozenLib)

# toml11
FetchContent_Declare(
    toml11
    URL https://github.com/ToruNiina/toml11/archive/refs/tags/v4.4.0.tar.gz
)
FetchContent_MakeAvailable(toml11)

# nlohmann::json
FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# tabulate
FetchContent_Declare(
    tabulate
    URL https://github.com/p-ranav/tabulate/archive/refs/tags/v1.5.tar.gz
)
FetchContent_MakeAvailable(tabulate)

# fmtlib
FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/11.1.4.tar.gz
)
FetchContent_MakeAvailable(fmt)

# asio - for crow library
find_package(Crow CONFIG REQUIRED)

add_executable(3ZCalculator
    "main.cpp"

    "src/zzz/enums.cpp"
    "src/zzz/stats.cpp"

    "src/zzz/combat/drive_disc_piece.cpp"

    "src/zzz/details/agent.cpp"
    "src/zzz/details/anomaly.cpp"
    "src/zzz/details/drive_disc_set.cpp"
    "src/zzz/details/rotation.cpp"
    "src/zzz/details/skill.cpp"
    "src/zzz/details/wengine.cpp"

    "src/calc/calculator.cpp"

    "src/backend/backend.cpp"
    "src/backend/logger.cpp"
    "src/backend/object_manager.cpp"
)

target_include_directories(3ZCalculator PRIVATE src)

target_link_libraries(3ZCalculator PRIVATE 
    frozen-headers
    toml11::toml11
    nlohmann_json::nlohmann_json
    tabulate::tabulate
    fmt::fmt
    Crow::Crow
    asio::asio
)

add_compile_definitions(DEBUG_STATUS)
