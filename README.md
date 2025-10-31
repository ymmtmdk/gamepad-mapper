# GamepadMapper - 複数ゲームパッド対応キーボードマッパー

複数のゲームパッドの入力を同時にキーボード入力に変換するWindows用アプリケーションです。DirectInput8を使用して複数のゲームパッドを個別に管理し、各デバイス固有の設定に基づいてキーボードの仮想キーコードを送信します。

## 🎮 主な機能

- **複数ゲームパッド同時対応**: 無制限のゲームパッドを同時使用可能
- **デバイス固有設定**: 各ゲームパッドに個別の設定ファイル
- **ホットプラグ対応**: 実行中の接続/切断を自動検出・再接続
- **統合ログシステム**: 全デバイスの状態を1つのログで管理
- **自動設定生成**: 新しいデバイスの設定ファイルを自動作成
- **デバイス識別**: ゲームパッド名、GUID、インスタンス名による区別

## 🏗️ アーキテクチャ

### 新しいモジュール構成

```
GamepadMapper/
├── WinMain.cpp                     # メインエントリーポイント
├── Application.{h,cpp}             # 複数ゲームパッド対応アプリケーション
├── MultipleGamepadManager.{h,cpp}  # 複数デバイス統合管理
├── GamepadDevice.{h,cpp}          # 個別デバイス管理
├── JsonConfigManager.{h,cpp}      # JSON設定ファイル管理
├── KeyResolver.{h,cpp}            # 文字列ベースのキー解決
├── InputProcessor.{h,cpp}         # 入力処理・キーボード変換
├── Logger.{h,cpp}                 # ログ管理・画面表示
├── WindowManager.{h,cpp}          # ウィンドウ管理・描画
└── gamepad_config_*.json          # デバイス固有設定ファイル
```

## ⚙️ 設定ファイルシステム

### ファイル命名規則

各ゲームパッドに対して個別の設定ファイルが自動生成されます：

```
gamepad_config_{device_name}.json
```

**例:**
- `gamepad_config_Xbox_Controller.json`
- `gamepad_config_PS4_Controller.json`
- `gamepad_config_Generic_USB_Gamepad.json`

## 📁 利用可能な設定ファイル

プロジェクトには、様々なデバイスと用途に応じた設定ファイルサンプルが用意されています：

### 🎮 デバイス別設定
- **`gamepad_config_xbox.json`** - Xbox One/Series Controller（標準設定）
- **`gamepad_config_ps4.json`** - PlayStation 4 Controller  
- **`gamepad_config_switch_pro.json`** - Nintendo Switch Pro Controller

### 🎯 用途別設定  
- **`gamepad_config_gaming.json`** - ゲーム用（高速応答・低遅延）
- **`gamepad_config_productivity.json`** - 作業効率化（Ctrl+キー組み合わせ）
- **`gamepad_config_media.json`** - メディア再生用（動画・音楽操作）

### 基本設定例
```json
{
  "device_info": {
    "name": "Xbox Controller",
    "instance_name": "Controller (Xbox One For Windows)",
    "guid": "{045e028e-0000-0000-0000-504944564944}"
  },
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["z"] },
      { "index": 1, "keys": ["x"] },
      { "index": 8, "keys": ["ctrl", "alt", "delete"] }
    ],
    "dpad": {
      "up": ["up"], "down": ["down"], 
      "left": ["left"], "right": ["right"]
    },
    "left_stick": {
      "left": ["a"], "right": ["d"], 
      "up": ["w"], "down": ["s"]
    }
  },
  "config": {
    "stick_threshold": 400,
    "log_level": "info",
    "enable_vibration": true,
    "deadzone_percentage": 10
  }
}
```

**詳細な設定方法は [設定ガイド](docs/CONFIGURATION_GUIDE.md) を参照してください。**

## 🎯 ゲームパッド入力変換

### サポートする入力
- **ボタン**: 128個のボタンマッピング対応
- **POV（方向パッド）**: 8方向対応（上下左右・斜め）
- **アナログスティック**: 左スティック（X/Y軸）のデジタル変換
- **閾値設定**: アナログスティックの感度調整可能

### キーマッピング機能
- **単一キー**: `["z"]`
- **複数キー同時押し**: `["ctrl", "alt", "delete"]`
- **特殊キー**: `printscreen`, `alt`, `tab`, `win`
- **VKコード直接指定**: `"65"`, `"0x41"` (10進数・16進数対応)

### キー名対応表
| キー名 | 説明 |
|---|---|
| `a`-`z`, `0`-`9` | アルファベット・数字 |
| `space` | スペースキー |
| `enter` | エンターキー |
| `escape` | エスケープキー |
| `ctrl`, `alt`, `shift` | 修飾キー |
| `win`, `lwin`, `rwin` | Windowsキー |
| `f1`-`f12` | ファンクションキー |
| `up`, `down`, `left`, `right` | 矢印キー |
| `printscreen`, `prtsc` | プリントスクリーンキー |
| `tab`, `backspace`, `delete` | その他特殊キー |

## 🛠️ ビルド方法

### 必要な環境
- **言語**: C++20
- **API**: Win32 API, DirectInput8
- **ビルド**: Visual Studio 2019+ / MinGW / CMake
- **依存ライブラリ**: `dinput8.lib`, `dxguid.lib`, `user32.lib`, `nlohmann/json`

### ビルド手順
Visual Studioによるビルドと、Linux上からのクロスコンパイルに対応しています。

Windows
```
CMakeプロジェクトとして開き、x64 Debug With Vcpkg構成を選択して、ビルド
```

Linux
```bash
bash build.sh
```

### 実行ファイル
ビルド成功後、`GamepadMapper.exe` が生成されます。

Windows
```
通常通り起動
```

Linux
```
wine build/GamepadMapper.exe
```

## 📋 技術詳細

### 主要クラス

| クラス | 責務 | 主要メソッド | ファイル |
|--------|------|------------|----------|
| `Application` | アプリケーション全体の制御・ライフサイクル管理 | `Initialize()`, `Run()`, `Shutdown()` | `src/Application.{h,cpp}` |
| `MultipleGamepadManager` | 複数ゲームパッド統合管理・デバイス検出 | `ScanForDevices()`, `ProcessAllDevices()` | `src/MultipleGamepadManager.{h,cpp}` |
| `GamepadDevice` | 個別デバイス管理・入力処理 | `LoadConfiguration()`, `ProcessInput()` | `src/GamepadDevice.{h,cpp}` |
| `JsonConfigManager` | JSON設定ファイル管理・設定読み込み | `load()`, `save()`, `getButtonKeys()` | `src/JsonConfigManager.{h,cpp}` |
| `InputProcessor` | 入力変換・キーボード送信・仮想キー生成 | `ProcessGamepadInput()`, `SendVirtualKey()` | `src/InputProcessor.{h,cpp}` |
| `KeyResolver` | キー名→仮想キーコード変換 | `resolve()`, `resolveSequence()` | `src/KeyResolver.{h,cpp}` |
| `WindowManager` | ウィンドウ管理・GUI描画 | `Init()`, `GetHwnd()`, `ShowWindow()` | `src/WindowManager.{h,cpp}` |
| `ModernLogger` | 高性能ログシステム（spdlogベース） | `info()`, `error()`, `debug()` | `src/ModernLogger.{h,cpp}` |

#### 主要な相互作用

```
Application
    ├── WindowManager          (GUI管理)
    ├── MultipleGamepadManager (デバイス統合管理)
    │   └── GamepadDevice[]    (個別デバイス)
    │       ├── JsonConfigManager (設定管理)
    │       ├── InputProcessor    (入力変換)
    │       └── KeyResolver       (キー解決)
    └── ModernLogger           (ログ出力)
```

## 📚 ドキュメント

詳細な情報については、以下のドキュメントを参照してください：

### 🚀 メインドキュメント（簡潔版・推奨）
- [API リファレンス](docs/API_REFERENCE.md) - 主要クラス・使用例・設定形式
- [開発者ガイド](docs/DEVELOPER_GUIDE.md) - クイックスタート・コーディング規約  
- [トラブルシューティング](docs/TROUBLESHOOTING.md) - よくある問題の解決法

### 📖 詳細版ドキュメント（必要時のみ）
- [API リファレンス（詳細版）](docs/API_REFERENCE_ORIGINAL.md) - 全API詳細説明
- [開発者ガイド（詳細版）](docs/DEVELOPER_GUIDE_ORIGINAL.md) - 理論的背景・高度な開発情報
- [トラブルシューティング（詳細版）](docs/TROUBLESHOOTING_ORIGINAL.md) - 網羅的問題解決ガイド

### 📊 プロジェクト分析資料
- [ドキュメント実装分析](docs/DOCUMENT_IMPLEMENTATION_ANALYSIS.md) - 整合性分析結果
- [ドキュメント簡略化レポート](docs/DOCUMENTATION_SIMPLIFICATION_REPORT.md) - 改善作業レポート
- [設定ガイド](docs/CONFIGURATION_GUIDE.md) - 詳細な設定方法・カスタマイズガイド
- [設定ファイル作成レポート](docs/CONFIGURATION_FILES_CREATION_REPORT.md) - 設定ファイル整備結果
