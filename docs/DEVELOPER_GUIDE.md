# GamepadMapper 開発者ガイド（簡潔版）

## 🚀 クイックスタート

### 環境要件
- **コンパイラ**: MinGW-w64 (GCC 9.0+) または Visual Studio 2019+
- **CMake**: 3.20+
- **vcpkg**: パッケージ管理

### セットアップ（3分）
```bash
git clone <repository-url>
cd GamepadMapper
bash build.sh        # 自動ビルド
./build/GamepadMapper.exe
```

---

## 🏗️ アーキテクチャ

```
Application (メイン)
├── WindowManager (GUI)
├── MultipleGamepadManager (デバイス統合)
│   └── GamepadDevice[] (個別デバイス)
│       ├── JsonConfigManager (設定)
│       ├── InputProcessor (入力変換)
│       └── KeyResolver (キー解決)
└── ModernLogger (ログ)
```

---

## 📝 コーディング規約

### 基本ルール
```cpp
// ✅ 良い例
class GamepadDevice {
private:
    std::wstring m_deviceName;        // メンバー変数は m_ プレフィックス
    bool m_connected = false;         // 初期化必須
};

// ✅ ログ使用
LOG_INFO("Device connected: {}", deviceName);
LOG_ERROR("Config load failed: {}", error);

// ✅ エラーハンドリング
if (FAILED(hr)) {
    LOG_ERROR("DirectInput error: 0x{:08X}", hr);
    return false;
}
```

### インクルード順序
```cpp
// 1. 対応ヘッダー
#include "GamepadDevice.h"
// 2. C++標準ライブラリ
#include <memory>
#include <string>
// 3. サードパーティ
#include <spdlog/spdlog.h>
// 4. Windows API（最後）
#include <windows.h>
```

---

## 🛠️ よくある開発タスク

### 新しいデバイス対応
```cpp
// 1. デバイス固有の設定ファイル作成
// gamepad_config_newdevice.json

// 2. 特殊処理が必要な場合
class NewGamepadDevice : public GamepadDevice {
    bool Initialize(/* ... */) override {
        // デバイス固有の初期化処理
        return GamepadDevice::Initialize(/* ... */);
    }
};
```

### カスタムキーマッピング
```json
{
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["ctrl", "c"] },  // 複数キー
      { "index": 1, "keys": ["f1"] }           // ファンクションキー
    ]
  }
}
```

### ログレベル設定
```cpp
// 開発時: DEBUG
LOG_SET_LEVEL(spdlog::level::debug);

// リリース時: INFO
LOG_SET_LEVEL(spdlog::level::info);
```

---

## 🔧 トラブルシューティング

### ビルドエラー
```bash
# vcpkg依存関係エラー
vcpkg install spdlog:x64-mingw-dynamic
vcpkg integrate install

# CMakeキャッシュクリア
rm -rf build/ && bash build.sh
```

### ランタイムエラー
```cpp
// DirectInput初期化失敗 → 管理者権限で実行
// デバイス認識しない → LOG_DEBUG でデバイス列挙確認
// 設定反映されない → 設定ファイルパス・権限確認
```

### デバッグ方法
```cpp
// 詳細ログ有効化
LOG_SET_LEVEL(spdlog::level::debug);

// 入力状態確認
LOG_DEBUG("Button[{}]: {}, Stick X:{} Y:{}", 
          index, pressed, js.lX, js.lY);
```

---

## 📚 重要ファイル

| ファイル | 用途 |
|----------|------|
| `src/Application.{h,cpp}` | メインアプリケーション |
| `src/GamepadDevice.{h,cpp}` | デバイス処理の中核 |
| `src/JsonConfigManager.{h,cpp}` | 設定管理 |
| `gamepad_config_*.json` | デバイス別設定 |
| `gamepad_mapper.log` | ログファイル |

---

## 🚨 注意点

- **管理者権限**: DirectInput使用時は管理者権限が必要な場合あり
- **設定ファイル**: デバイス名に基づいて自動選択される
- **ログファイル**: 実行ディレクトリに出力される
- **メモリ管理**: スマートポインタを積極的に使用

---

**より詳細な情報は元の `DEVELOPER_GUIDE.md` または API リファレンスを参照してください。**