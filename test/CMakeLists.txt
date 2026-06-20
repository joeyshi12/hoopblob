cmake_minimum_required(VERSION 3.16)
project(hoopblob_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the deterministic game logic against the host-side bn::fixed shim.
add_executable(hoopblob_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ball.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_physics.cpp
)

target_include_directories(hoopblob_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/butano
)

enable_testing()
add_test(NAME physics COMMAND hoopblob_tests)
