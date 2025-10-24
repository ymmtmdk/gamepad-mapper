#!/bin/bash

echo "Building Gamepad Mapper..."

# ビルドディレクトリを作成
mkdir -p build
cd build

# CMakeを実行
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake -G Ninja

# ビルド実行
ninja

