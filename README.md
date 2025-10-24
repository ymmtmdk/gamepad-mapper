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

### アーキテクチャの改善

**旧システム (単一デバイス)**:
```
Application → DirectInputManager (1デバイス)
           → JsonConfigManager (1設定)
           → InputProcessor (1処理)
```

**新システム (複数デバイス)**:
```
Application → MultipleGamepadManager
           ├── GamepadDevice[0] → JsonConfigManager[0] → InputProcessor[0]
           ├── GamepadDevice[1] → JsonConfigManager[1] → InputProcessor[1]
           └── GamepadDevice[n] → JsonConfigManager[n] → InputProcessor[n]
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

### 設定ファイル例

```json
{
  "device_info": {
    "name": "Xbox Controller",
    "instance_name": "Controller (Xbox One For Windows)",
    "guid": "{12345678-1234-1234-1234-123456789ABC}"
  },
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["z"] },
      { "index": 1, "keys": ["x"] },
      { "index": 8, "keys": ["ctrl", "alt", "delete"] }
    ],
    "dpad": {
      "up": ["up"],
      "down": ["down"],
      "left": ["left"],
      "right": ["right"]
    },
    "left_stick": {
      "left": ["a"],
      "right": ["d"],
      "up": ["w"],
      "down": ["s"]
    }
  },
  "config": {
    "stick_threshold": 400,
    "log_level": "info"
  }
}
```

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

```bash
# ビルドスクリプトを使用（推奨）
bash build.sh

# または手動ビルド
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-toolchain.cmake
make GamepadMapper
```

### 実行ファイル
ビルド成功後、`GamepadMapper.exe` が生成されます。

## 🚀 使用方法

### 基本的な使用手順

1. **アプリケーション起動**
   ```bash
   ./GamepadMapper.exe
   ```

2. **ゲームパッド接続**
   - USB/Bluetooth経由でゲームパッドを接続
   - アプリケーションが自動的にデバイスを検出
   - 各デバイス用の設定ファイルが自動生成

3. **設定カスタマイズ**
   - 生成された `gamepad_config_{device_name}.json` を編集
   - ボタンやスティックのキーマッピングを変更
   - アプリケーションを再起動して設定を反映

4. **複数デバイス使用**
   - 複数のゲームパッドを同時接続
   - 各デバイスが独立した設定で動作
   - リアルタイムでの接続/切断対応

### ログ出力例

```
Gamepad Status: 2/2 devices connected
  • Connected: Xbox Controller
  • Connected: PS4 Controller
[Xbox Controller] Button0 -> z PRESSED
[PS4 Controller] Button1 -> x PRESSED
[Xbox Controller] Button0 -> z RELEASED
```

## 📋 技術詳細

### 主要クラス

#### GamepadDevice
- 個別のゲームパッドデバイス管理
- DirectInputデバイスの初期化・設定
- デバイス固有の設定ファイル管理
- 接続/切断の検出と再接続

#### MultipleGamepadManager
- 複数デバイスの統合管理
- デバイスの自動検出・列挙
- 定期的なデバイススキャン (5秒間隔)
- GUID/名前によるデバイス検索

#### Application
- 複数ゲームパッド対応のメインアプリケーション
- MultipleGamepadManagerの統合
- 複数デバイス状態のログ表示
- 統合されたウィンドウ管理

### 動的デバイス管理

- **ホットプラグ対応**: 実行中の接続/切断を自動検出
- **自動再接続**: 切断されたデバイスの再接続を試行
- **デバイス状態監視**: リアルタイムでの接続状態表示

### パフォーマンス最適化

- **効率的なデバイススキャン**: 5秒間隔での新デバイス検出
- **状態変化検出**: 前フレームとの比較による最適化
- **メモリ管理**: RAII パターンによる安全なリソース管理

## 🔧 トラブルシューティング

### よくある問題

1. **デバイスが認識されない**
   - Windowsのデバイスマネージャーでゲームパッドが認識されているか確認
   - アプリケーション再起動でデバイススキャンを再実行

2. **設定が反映されない**
   - 設定ファイルのJSON形式が正しいか確認
   - アプリケーション再起動が必要

3. **複数デバイスで競合する**
   - 各デバイスの設定ファイルで異なるキーマッピングを設定
   - ログでどのデバイスからの入力かを確認

### ログファイル

詳細な動作ログは `multi_gamepad_mapper.log` に出力されます。

## 📝 ライセンス

このプロジェクトはオープンソースソフトウェアです。

## 🤝 貢献

バグ報告や機能要望は、プロジェクトのIssueトラッカーまでお願いします。