# rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake
make

