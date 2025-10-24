# DirectInput ゲームパッド to キーボード マッパー

DirectInput8を使用してゲームパッドの入力をキーボードイベントに変換するWindowsアプリケーションです。**モダンなJSON設定ファイル**による柔軟なキーマッピング、複数キー同時押しシーケンスを提供します。

## 🎯 主要機能

### ✅ ゲームパッド入力変換
- **ボタン**: 128個のボタンマッピング対応
- **POV（方向パッド）**: 8方向対応（上下左右・斜め）
- **アナログスティック**: 左スティック（X/Y軸）のデジタル変換
- **閾値設定**: アナログスティックの感度調整可能

### ✅ 高度なキーマッピング (JSONベース)
- **単一キー**: `["Z"]`
- **複数キー同時押し**: `["CTRL", "ALT", "DEL"]`
- **特殊キー**: `printscreen`, `alt`, `tab`, `win`
- **VKコード直接指定**: `"65"`, `"0x41"` (10進数・16進数対応)

## 🏗️ アーキテクチャ

### モジュール構成

```
WindowsProject1/
├── WinMain.cpp                # メインエントリーポイント
├── JsonConfigManager.{h,cpp}  # ✅ JSON設定ファイル管理 (nlohmann/json)
├── KeyResolver.{h,cpp}        # ✅ 文字列ベースのキー解決システム
├── DirectInputManager.{h,cpp} # ✅ DirectInput初期化・デバイス管理
├── InputProcessor.{h,cpp}     # ✅ 入力処理・キーボード変換
├── Logger.{h,cpp}             # ✅ ログ管理・画面表示
├── WindowManager.{h,cpp}      # ✅ ウィンドウ管理・描画
└── gamepad_config.json        # 設定ファイル（自動生成）
```

### 技術スタック
- **言語**: C++20
- **API**: Win32 API, DirectInput8
- **ビルド**: Visual Studio 2019+ / MinGW / CMake
- **依存ライブラリ**: `dinput8.lib`, `dxguid.lib`, `user32.lib`, **nlohmann/json**

## 🔧 実装詳細

### JsonConfigManager & KeyResolver モジュール
**責務**: JSON設定の読み書き、キーマッピングのコンパイルと解決

```cpp
// モダンなキー名 → VKコード マッピング
class KeyResolver {
public:
    static std::optional<WORD> resolve(const std::string& keyName);
    static std::vector<WORD> resolveSequence(const std::vector<std::string>& keys);
};

// JSON設定管理
class JsonConfigManager {
public:
    bool load(const std::string& path = "gamepad_config.json");
    std::vector<WORD> getButtonKeys(int buttonIndex) const;
    std::vector<WORD> getDpadKeys(const std::string& direction) const;
};
```

**主要機能**:
- `gamepad_config.json`の自動生成・読み込み
- 直感的なJSONスキーマ (`["CTRL", "ALT", "DEL"]`)
- 豊富なキー名対応（`printscreen`, `win`, `ctrl`など小文字中心）
- パフォーマンス向上のためのキーマッピングキャッシュ

### DirectInputManager モジュール
**責務**: DirectInputデバイスの初期化・管理
- ゲームコントローラー自動検出
- デバイス設定（軸範囲: -1000〜1000）
- バックグラウンド非排他モード

### InputProcessor モジュール
**責務**: ゲームパッド状態のキーボードイベント変換
- 状態変化検出（前フレームとの比較）
- 正しいキー押下/解放順序制御
- `SendInput` APIによる確実なキー送信

## ⚙️ 設定ファイル

### JSONファイル例 (`gamepad_config.json`)
```json
{
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

### キー名対応表 (小文字推奨)
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

```bash
# ビルドスクリプトを使用（推奨）
bash b.sh
```

