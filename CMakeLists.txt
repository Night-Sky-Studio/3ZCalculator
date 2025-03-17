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
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG        v4.4.0
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(toml11)

# tabulate
FetchContent_Declare(
    tabulate
    GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
    GIT_TAG        v1.5
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(tabulate)

# fmtlib - 10.2.1
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.1.4
)
FetchContent_MakeAvailable(fmt)

# asio - for crow library
IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using vcpkg
    find_package(Crow CONFIG REQUIRED)
ENDIF()

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

    "src/calculator/calculator.cpp"
    "src/calculator/converters.cpp"

    "src/backend/backend.cpp"
    "src/backend/object_manager.cpp"
)

target_include_directories(3ZCalculator PRIVATE src)

target_link_libraries(3ZCalculator PRIVATE 
    frozen-headers
    toml11::toml11
    tabulate::tabulate
    fmt::fmt
    Crow::Crow
    asio::asio
)

#add_compile_definitions(DEBUG_STATUS)
