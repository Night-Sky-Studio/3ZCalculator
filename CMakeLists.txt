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
#set(DCMAKE_TOOLCHAIN_FILE "D:/lib/vcpkg/scripts/buildsystems/vcpkg.cmake")

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

    "src/library/cached_memory.cpp"
    "src/library/logger.cpp"
    "src/library/rpn.cpp"
    
    #"src/zzz/stats/basic.cpp"
    #"src/zzz/stats/grid.cpp"

    "src/zzz/details/agent.cpp"
    "src/zzz/details/anomaly.cpp"
    "src/zzz/details/ddp.cpp"
    "src/zzz/details/dds.cpp"
    "src/zzz/details/rotation.cpp"
    "src/zzz/details/skill.cpp"
    "src/zzz/details/wengine.cpp"

    "src/calc/calculator.cpp"

    "src/backend/backend.cpp"
 "src/zzz/stats/regular.cpp" "src/zzz/stats/grid.cpp" "src/zzz/stats/relative.cpp")

target_include_directories(3ZCalculator PRIVATE src)

target_link_libraries(3ZCalculator PRIVATE 
    frozen-headers
    tabulate::tabulate
    fmt::fmt
    Crow::Crow
    asio::asio
)

add_compile_definitions(DEBUG_STATUS)
add_compile_definitions(ADDITIONAL_CHECK_MODE)
