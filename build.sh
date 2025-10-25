#!/bin/bash

echo "Building Gamepad Mapper..."

# ビルドディレクトリを作成
mkdir -p build
cd build

# CMakeの再実行が必要かチェック
# CMakeCache.txtが存在しない、またはCMakeLists.txtかmingw-w64-toolchain.cmakeが更新されている場合にcmakeを実行
if [ ! -f "CMakeCache.txt" ] || [ "../CMakeLists.txt" -nt "CMakeCache.txt" ] || [ "../mingw-w64-toolchain.cmake" -nt "CMakeCache.txt" ]; then
  echo "Running CMake..."
  cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake -G Ninja
else
  echo "CMake configuration is up to date. Skipping."
fi

# ビルド実行
ninja

