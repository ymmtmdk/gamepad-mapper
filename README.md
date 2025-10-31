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
├── GamepadManager.{h,cpp}  # 複数デバイス統合管理
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

## 📁 設定ファイル

プロジェクトには、様々なデバイスと用途に応じた設定ファイルサンプルが `config/samples/` フォルダに用意されています：

### 🎮 デバイス別設定
- **`config/samples/gamepad_config_xbox.json`** - Xbox One/Series Controller（標準設定）
- **`config/samples/gamepad_config_ps4.json`** - PlayStation 4 Controller  
- **`config/samples/gamepad_config_switch_pro.json`** - Nintendo Switch Pro Controller

### 🎯 用途別設定  
- **`config/samples/gamepad_config_gaming.json`** - ゲーム用（高速応答・低遅延）
- **`config/samples/gamepad_config_productivity.json`** - 作業効率化（Ctrl+キー組み合わせ）
- **`config/samples/gamepad_config_media.json`** - メディア再生用（動画・音楽操作）

### 🚀 使用方法
```bash
# 1. 適切なサンプルをコピー
cp config/samples/gamepad_config_xbox.json gamepad_config_mydevice.json

# 2. デバイス情報を実際の値に更新
# 3. 好みに応じてキー配置をカスタマイズ
```

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

**詳細な設定方法は [設定ガイド](docs/CONFIGURATION_GUIDE.md) または [config/README.md](config/README.md) を参照してください。**

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

## 📚 ドキュメント

詳細な情報については、以下のドキュメントを参照してください：

### 🚀 メインドキュメント
- [API リファレンス](docs/API_REFERENCE.md) - 主要クラス・使用例・設定形式
