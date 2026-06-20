#!/usr/bin/env bash
# Build and run the Hoopblob host-side test suite.
# Requires only a C++ compiler (clang++ or g++) — no GBA toolchain needed.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-c++}"

"$CXX" -std=c++20 -O2 -Wall -Wextra \
    -I "$ROOT/include" \
    -I "$ROOT/test/butano" \
    "$ROOT/src/entity.cpp" \
    "$ROOT/src/ball.cpp" \
    "$ROOT/test/test_physics.cpp" \
    -o "$ROOT/test/hoopblob_tests"

"$ROOT/test/hoopblob_tests"
