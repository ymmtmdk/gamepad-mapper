#!/bin/bash

# 複数ゲームパッド対応版のビルドスクリプト

echo "Building Multiple Gamepad Mapper..."

# ビルドディレクトリを作成
mkdir -p build_multiple
cd build_multiple

# CMakeを実行
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake

# ビルド実行
make MultiGamepadMapper

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable: MultiGamepadMapper.exe"
    
    # 実行ファイルをメインディレクトリにコピー
    cp MultiGamepadMapper.exe ../
    
    echo "MultiGamepadMapper.exe copied to main directory."
else
    echo "Build failed!"
    exit 1
fi