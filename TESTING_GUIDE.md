# ユニットテスト導入ガイド

このドキュメントは、プロジェクトにユニットテストを導入するための手順をまとめたものです。

## 1. 方針

- **テストフレームワーク**: Google Test (gtest) を使用します。
- **テスト対象**: GUIに依存しないビジネスロジック（設定管理、入力処理など）を主対象とします。
- **構成**: テスト対象のソースコードを静的ライブラリとして分離し、メインアプリケーションとテストプログラムの両方から利用できるようにリファクタリングします。

---

## 2. 導入手順

### 手順 1: 依存関係の追加

`vcpkg.json` に `gtest` を追加し、Google Testフレームワークをプロジェクトに導入します。

**変更前の `vcpkg.json`:**
```json
{
 "name": "cmakeproject1",
 "version": "0.1.0",
 "dependencies": [
 "nlohmann-json"
 ]
}
```

**変更後の `vcpkg.json`:**
```json
{
 "name": "cmakeproject1",
 "version": "0.1.0",
 "dependencies": [
 "nlohmann-json",
 "gtest"
 ]
}
```
変更後、vcpkg が自動的に `gtest` をインストールします。

### 手順 2: CMakeLists.txt のリファクタリング

`CMakeLists.txt` を編集し、ロジック部分を静的ライブラリ `gamepadlogic` として分離します。

1.  **静的ライブラリの定義**:
    以下のコードを `add_executable` の前に追加します。これにより、テスト可能にしたいソースコードが `gamepadlogic` というライブラリにまとめられます。

    ```cmake
    # ビジネスロジックを静的ライブラリとして分離
    add_library(gamepadlogic STATIC
        src/KeyResolver.cpp
        src/JsonConfigManager.cpp
        src/InputProcessor.cpp
        src/Logger.cpp
        src/GamepadDevice.cpp
        src/MultipleGamepadManager.cpp
    )

    # ライブラリに必要なインクルードディレクトリを設定
    target_include_directories(gamepadlogic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    
    # ライブラリに必要な依存関係をリンク
    target_link_libraries(gamepadlogic PRIVATE nlohmann_json::nlohmann_json)
    ```

2.  **メイン実行ファイルの更新**:
    既存の `add_executable` を修正し、ソースファイルのリストを削減して、代わりに `gamepadlogic` ライブラリをリンクします。

    ```cmake
    # 実行ファイルの定義を更新
    add_executable (GamepadMapper WIN32
        src/Application.cpp
        src/WindowManager.cpp
        src/WinMain.cpp
        src/GamepadMapper.rc
    )

    # 静的ライブラリと他の依存関係をリンク
    target_link_libraries(GamepadMapper PRIVATE 
        gamepadlogic 
        dinput8 
        dxguid 
        user32 
        gdi32
    )
    ```

3.  **テストの設定を追加**:
    `CMakeLists.txt` の末尾に、テストを有効化し、テスト用実行ファイルをビルドするための設定を追加します。

    ```cmake
    # ===== テスト設定 =====
    if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        enable_testing()

        # gtestパッケージを探す
        find_package(GTest REQUIRED)
        include(GoogleTest)

        # テスト用の実行ファイルを追加
        add_executable(unit_tests
            tests/KeyResolver.test.cpp
            # 他のテストファイルもここに追加
        )

        # テスト実行ファイルにライブラリをリンク
        target_link_libraries(unit_tests PRIVATE 
            gamepadlogic
            GTest::gtest
            GTest::gtest_main
        )

        # CTestにテストを自動検出させる
        gtest_discover_tests(unit_tests)
    endif()
    ```

### 手順 3: サンプルテストの作成

1.  プロジェクトのルートに `tests` ディレクトリを作成します。
2.  `tests` ディレクトリ内に、`KeyResolver.test.cpp` という名前で新しいファイルを作成し、以下の内容を記述します。

    ```cpp
    #include "gtest/gtest.h"
    #include "KeyResolver.h" // テスト対象のヘッダー

    // KeyResolver::resolve が正常にキーコードを解決できるかをテスト
    TEST(KeyResolverTest, ResolvesValidKey) {
        // "enter" は 0x0D (VK_RETURN) に解決されるはず
        std::optional<WORD> result = KeyResolver::resolve("enter");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), VK_RETURN);
    }

    // KeyResolver::resolve が無効なキー名を処理できるかをテスト
    TEST(KeyResolverTest, HandlesInvalidKey) {
        // "invalid_key" は解決できないはず
        std::optional<WORD> result = KeyResolver::resolve("invalid_key");
        EXPECT_FALSE(result.has_value());
    }

    // KeyResolver::resolveSequence が複数のキーを解決できるかをテスト
    TEST(KeyResolverTest, ResolvesValidSequence) {
        std::vector<std::string> keys = {"ctrl", "c"};
        std::vector<WORD> expected = {VK_CONTROL, 0x43}; // 0x43 は 'C'
        std::vector<WORD> result = KeyResolver::resolveSequence(keys);
        EXPECT_EQ(result, expected);
    }
    ```

---

## 3. ビルドとテストの実行

上記の手順が完了したら、以下のコマンドでプロジェクトをビルドし、テストを実行します。

1.  **CMakeの構成 (vcpkg toolchain を指定):**
    ```sh
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkgのルートパス]/scripts/buildsystems/vcpkg.cmake
    ```
    *`[vcpkgのルートパス]` は実際のvcpkgのインストール先に置き換えてください。*

2.  **ビルド:**
    ```sh
    cmake --build build
    ```

3.  **テストの実行:**
    ビルドディレクトリに移動して `ctest` を実行します。
    ```sh
    cd build
    ctest --output-on-failure
    ```

    テストが成功すると、以下のような出力が表示されます。
    ```
    100% tests passed, 0 tests failed out of 3

    Total Test time (real) =   0.01 s
    ```
