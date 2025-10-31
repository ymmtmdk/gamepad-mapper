# GamepadMapper API リファレンス（簡潔版）

## 🎯 クイックリファレンス

### 主要クラス一覧

| クラス | 用途 | 主要メソッド |
|--------|------|------------|
| `Application` | アプリ制御 | `Initialize()`, `Run()`, `Shutdown()` |
| `MultipleGamepadManager` | デバイス管理 | `ScanForDevices()`, `ProcessAllDevices()` |
| `GamepadDevice` | 個別デバイス | `Initialize()`, `ProcessInput()` |
| `JsonConfigManager` | 設定管理 | `load()`, `save()`, `getButtonKeys()` |
| `InputProcessor` | 入力変換 | `ProcessGamepadInput()`, `SendVirtualKey()` |
| `KeyResolver` | キー解決 | `resolve()`, `resolveSequence()` |
| `ModernLogger` | ログ出力 | `info()`, `error()`, `debug()` |

---

## 🚀 基本的な使用例

### アプリケーション初期化
```cpp
Application app;
if (!app.Initialize(hInstance, nCmdShow)) {
    LOG_ERROR("Initialization failed");
    return -1;
}
```

### デバイス処理
```cpp
auto device = std::make_shared<GamepadDevice>();
device->Initialize(pDirectInput, &deviceInstance, hWnd);
device->ProcessInput(); // メインループ内で実行
```

### 設定ファイル操作
```cpp
JsonConfigManager config("gamepad_config_xbox.json");
if (config.load()) {
    auto keys = config.getButtonKeys(0); // ボタン0のキー取得
}
```

### ログ出力
```cpp
// 基本ログ（推奨）
LOG_INFO("Device connected: {}", deviceName);
LOG_ERROR("Configuration load failed");
LOG_DEBUG("Button state: {}", buttonState);
LOG_WARN("Config file outdated: {}", filename);

// ワイド文字ログ（日本語対応）
LOG_INFO_W(L"デバイス接続: Xbox Controller");
LOG_ERROR_W(L"設定ファイル読み込みエラー");

// フレームログ（リアルタイム表示用）
FRAME_LOG_CLEAR();
FRAME_LOG_APPEND(L"Button[0]: Pressed");
FRAME_LOG_GAMEPAD_INFO(true, L"Xbox Controller", L"Controller 1");

// ログレベル設定
LOG_SET_LEVEL(spdlog::level::debug);  // trace, debug, info, warn, error
LOG_ENABLE_CONSOLE(true);             // コンソール出力有効化
```

---

## ⚙️ 設定ファイル形式

```json
{
  "device_info": {
    "name": "Xbox Controller",
    "instance_name": "Controller (Xbox One For Windows)"
  },
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["z"] },
      { "index": 1, "keys": ["x"] }
    ],
    "dpad": {
      "up": ["up"], "down": ["down"],
      "left": ["left"], "right": ["right"]
    },
    "left_stick": {
      "up": ["w"], "down": ["s"],
      "left": ["a"], "right": ["d"]
    }
  },
  "config": {
    "stick_threshold": 400,
    "log_level": "info"
  }
}
```

---

## 🔧 よく使う機能

### エラーハンドリング
```cpp
try {
    // 処理
} catch (const std::exception& e) {
    LOG_ERROR("Error: {}", e.what());
    return false;
}
```

### DirectInput初期化
```cpp
HRESULT hr = device->Initialize(pDirectInput, &deviceInstance, hWnd);
if (FAILED(hr)) {
    LOG_ERROR("Device init failed: 0x{:08X}", hr);
}
```

### 設定の動的変更
```cpp
// 新しい設定を適用
config.setConfig(newGamepadConfig, newSystemConfig);
config.save();
```

---

**詳細なAPIドキュメントが必要な場合は、元の `API_REFERENCE.md` を参照してください。**