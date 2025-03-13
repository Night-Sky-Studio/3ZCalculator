cmake_minimum_required(VERSION 3.28)

project(
    3ZCalculator
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "Calculates damage of ZZZ character with rough approximation"
    HOMEPAGE_URL ""
)
set(CMAKE_CXX_STANDARD 20)

# FetchContent

include(FetchContent)

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
    "src/backend/adaptors.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET 3ZCalculator PROPERTY CXX_STANDARD 20)
endif ()

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
