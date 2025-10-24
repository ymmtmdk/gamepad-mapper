#!/bin/bash

# ゲームパッドマッパーのビルドスクリプト (複数デバイス対応版)

echo "Building Gamepad Mapper..."

# ビルドディレクトリを作成
mkdir -p build
cd build

# CMakeを実行
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake

# ビルド実行
make GamepadMapper

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable: GamepadMapper.exe"
    
    # 実行ファイルをメインディレクトリにコピー
    cp GamepadMapper.exe ../
    
    echo "GamepadMapper.exe copied to main directory."
else
    echo "Build failed!"
    exit 1
fi