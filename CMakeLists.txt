# CMakeList.txt : CMakeProject1 の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required(VERSION 3.10)
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
# 元の単一ゲームパッド版
add_executable (CMakeProject1 WIN32  
  Application.cpp
  KeyResolver.cpp
  JsonConfigManager.cpp
  Logger.cpp
  WindowManager.cpp
  DirectInputManager.cpp
  InputProcessor.cpp
  WinMain.cpp)

# 新しい複数ゲームパッド対応版
add_executable (MultiGamepadMapper WIN32
  Application_Multiple.cpp
  GamepadDevice.cpp
  MultipleGamepadManager.cpp
  KeyResolver.cpp
  JsonConfigManager.cpp
  Logger.cpp
  WindowManager.cpp
  InputProcessor.cpp
  WinMain_Multiple.cpp)

# nlohmann_json を検索。vcpkg を利用している場合は vcpkg の toolchain を指定して CMake を呼んでください
find_package(nlohmann_json CONFIG QUIET)

# Unicodeを有効にする
add_compile_definitions(UNICODE _UNICODE)

target_link_options(CMakeProject1 PRIVATE -municode)
target_link_options(MultiGamepadMapper PRIVATE -municode)
if(MINGW)
  # 静的リンクを設定 
  target_link_options(CMakeProject1 PRIVATE -static)
  target_link_options(MultiGamepadMapper PRIVATE -static)
endif()

target_link_libraries(CMakeProject1 PRIVATE dinput8 dxguid user32 gdi32)
target_link_libraries(CMakeProject1 PRIVATE nlohmann_json::nlohmann_json)

# 複数ゲームパッド版のライブラリリンク
target_link_libraries(MultiGamepadMapper PRIVATE dinput8 dxguid user32 gdi32)
target_link_libraries(MultiGamepadMapper PRIVATE nlohmann_json::nlohmann_json)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
  set_property(TARGET MultiGamepadMapper PROPERTY CXX_STANDARD 20)
endif()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。

