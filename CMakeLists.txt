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

# 実行ファイルを WIN32（サブシステム:Windows）として作成
# 複数ゲームパッド対応版 (標準版)
add_executable (GamepadMapper WIN32  
  src/Application.cpp
  src/GamepadDevice.cpp
  src/MultipleGamepadManager.cpp
  src/KeyResolver.cpp
  src/JsonConfigManager.cpp
  src/Logger.cpp
  src/WindowManager.cpp
  src/InputProcessor.cpp
  src/WinMain.cpp
  src/GamepadMapper.rc
)

# nlohmann_json を検索。vcpkg を利用している場合は vcpkg の toolchain を指定して CMake を呼んでください
find_package(nlohmann_json CONFIG QUIET)

# Unicodeを有効にする
add_compile_definitions(UNICODE _UNICODE)

target_link_options(GamepadMapper PRIVATE -municode)
if(MINGW)
  # 静的リンクを設定 
  target_link_options(GamepadMapper PRIVATE -static)
endif()

target_link_libraries(GamepadMapper PRIVATE dinput8 dxguid user32 gdi32)
target_link_libraries(GamepadMapper PRIVATE nlohmann_json::nlohmann_json)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GamepadMapper PROPERTY CXX_STANDARD 20)
endif()

