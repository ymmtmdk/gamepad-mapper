# CMakeList.txt : CMakeProject1 の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required(VERSION 3.15)
project(CMakeProject1 LANGUAGES CXX)

# C++20 を指定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# サポートされている場合は、MSVC コンパイラのホット リロードを有効にします。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_definitions(UNICODE _UNICODE)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# 実行ファイルを WIN32（サブシステム:Windows）として作成
add_executable (GamepadMapper WIN32  
  src/Application.cpp
  src/GamepadDevice.cpp
  src/GamepadManager.cpp
  src/KeyResolver.cpp
  src/JsonConfigManager.cpp
  src/Logger.cpp
  src/DisplayBuffer.cpp
  src/WindowManager.cpp
  src/InputProcessor.cpp
  src/WinMain.cpp
  src/GamepadMapper.rc
)

target_link_options(GamepadMapper PRIVATE -municode)
if(MINGW)
  target_link_options(GamepadMapper PRIVATE -static)
endif()

target_link_libraries(GamepadMapper PRIVATE 
  dinput8 dxguid user32 gdi32
  nlohmann_json::nlohmann_json
  fmt::fmt
  spdlog::spdlog
)

set_property(TARGET GamepadMapper PROPERTY CXX_STANDARD 20)

