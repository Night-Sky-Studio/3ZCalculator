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

# FrozenLib
FetchContent_Declare(
    FrozenLib
    GIT_REPOSITORY https://github.com/serge-sans-paille/frozen
    GIT_TAG master
)
FetchContent_MakeAvailable(FrozenLib)

# toml11
FetchContent_Declare(
    toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG v4.4.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(toml11)

# tabulate
FetchContent_Declare(
    tabulate
    GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
    GIT_TAG v1.5
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tabulate)

# fmtlib - 10.2.1
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

add_executable(3ZCalculator
    "main.cpp"

    "src/zzz/enums.cpp"
    "src/zzz/stats.cpp"

    "src/zzz/details/agent.cpp"
    "src/zzz/details/anomaly.cpp"
    "src/zzz/details/drive_disc_set.cpp"
    "src/zzz/combat/drive_disc_piece.cpp"
    "src/zzz/details/skill.cpp"
    "src/zzz/details/wengine.cpp"

    "src/backend/calculator.cpp"
    "src/backend/converters.cpp"
 "src/backend/object_manager.cpp")

target_include_directories(3ZCalculator PRIVATE
    src
    src/backend
    src/library
    src/zzz
    src/zzz/combat
    src/zzz/details
)

target_link_libraries(3ZCalculator PRIVATE frozen-headers)
target_link_libraries(3ZCalculator PRIVATE toml11::toml11)
target_link_libraries(3ZCalculator PRIVATE tabulate::tabulate)
target_link_libraries(3ZCalculator PRIVATE fmt::fmt)
