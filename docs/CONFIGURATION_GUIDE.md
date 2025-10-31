# GamepadMapper 設定ガイド

## 📁 設定ファイル一覧

GamepadMapperには、様々な用途に応じた設定ファイルサンプルが用意されています：

### 🎮 デバイス別設定

| ファイル | 対象デバイス | 特徴 |
|----------|-------------|------|
| `gamepad_config_xbox.json` | Xbox One/Series Controller | 標準的なボタン配置、振動対応 |
| `gamepad_config_ps4.json` | PlayStation 4 Controller | PSボタン配置に最適化、低デッドゾーン |
| `gamepad_config_switch_pro.json` | Nintendo Switch Pro Controller | Nintendo配置、高感度設定 |

### 🎯 用途別設定

| ファイル | 用途 | 特徴 |
|----------|------|------|
| `gamepad_config_gaming.json` | ゲーム用 | 高速応答、ゲーム向けキー配置 |
| `gamepad_config_productivity.json` | 作業効率化 | Ctrl+キー組み合わせ、オフィス作業最適化 |
| `gamepad_config_media.json` | メディア再生 | 動画・音楽再生用、音量・シーク操作 |

---

## ⚙️ 設定ファイル構造詳細

### 基本構造
```json
{
  "device_info": {
    "name": "デバイス名",
    "instance_name": "Windowsでの表示名",
    "guid": "デバイス固有GUID"
  },
  "gamepad": {
    "buttons": [...],     // ボタン配置
    "dpad": {...},        // 十字キー
    "left_stick": {...},  // 左スティック
    "right_stick": {...}  // 右スティック
  },
  "config": {
    "stick_threshold": 400,        // スティック感度閾値
    "log_level": "info",           // ログレベル
    "enable_vibration": true,      // 振動有効化
    "deadzone_percentage": 10      // デッドゾーン(%）
  }
}
```

### ボタン設定詳細
```json
"buttons": [
  { "index": 0, "keys": ["z"] },              // 単一キー
  { "index": 1, "keys": ["ctrl", "c"] },      // 組み合わせキー
  { "index": 2, "keys": ["alt", "f4"] }       // 複数修飾キー
]
```

### スティック設定詳細
```json
"left_stick": {
  "left": ["a"],          // 左方向
  "right": ["d"],         // 右方向  
  "up": ["w"],            // 上方向
  "down": ["s"]           // 下方向
}
```

---

## 🎯 用途別詳細設定

### 🎮 ゲーム用設定（gamepad_config_gaming.json）
```json
{
  "特徴": "高速応答・低遅延",
  "用途": "アクションゲーム、FPS、格闘ゲーム",
  "最適化": {
    "stick_threshold": 300,     // 高感度
    "deadzone_percentage": 5,   // 低デッドゾーン
    "enable_vibration": true    // フィードバック有効
  },
  "推奨ボタン配置": {
    "A": "スペース（ジャンプ）",
    "B": "Ctrl（しゃがみ）", 
    "X": "Shift（走る）",
    "Y": "Alt（特殊動作）"
  }
}
```

### 💼 生産性向上設定（gamepad_config_productivity.json）
```json
{
  "特徴": "オフィス作業最適化",
  "用途": "文書編集、ブラウジング、プレゼンテーション",
  "最適化": {
    "stick_threshold": 500,     // 精密操作
    "deadzone_percentage": 15,  // 誤操作防止
    "enable_vibration": false   // 静音
  },
  "推奨ボタン配置": {
    "A": "Ctrl+C（コピー）",
    "B": "Ctrl+V（貼り付け）",
    "X": "Ctrl+Z（取り消し）",
    "Y": "Ctrl+Y（やり直し）"
  }
}
```

### 🎵 メディア再生設定（gamepad_config_media.json）
```json
{
  "特徴": "動画・音楽再生最適化",
  "用途": "YouTube、Netflix、音楽プレーヤー",
  "最適化": {
    "stick_threshold": 400,     // 標準感度
    "deadzone_percentage": 10,  // 標準デッドゾーン
    "enable_vibration": false   // 静音
  },
  "推奨ボタン配置": {
    "A": "スペース（再生/停止）",
    "B": "K（一時停止）",
    "X": "J（10秒戻る）",
    "Y": "L（10秒進む）"
  }
}
```

---

## 🔧 設定カスタマイズ

### 有効なキー名一覧
```json
// 文字キー
["a", "b", "c", ..., "z"]
["0", "1", "2", ..., "9"]

// 修飾キー
["ctrl", "alt", "shift", "win"]

// ファンクションキー
["f1", "f2", "f3", ..., "f12"]

// 特殊キー
["space", "enter", "escape", "tab", "backspace"]
["up", "down", "left", "right"]
["home", "end", "page_up", "page_down"]
["insert", "delete"]

// 数値キー
["plus", "minus", "multiply", "divide"]

// 記号
["comma", "period", "semicolon", "quote"]
```

### 感度調整パラメータ
```json
{
  "config": {
    "stick_threshold": 400,        // 100-1000（低=高感度）
    "deadzone_percentage": 10,     // 0-50（低=敏感）
    "log_level": "info",          // trace, debug, info, warn, error
    "enable_vibration": true      // true/false
  }
}
```

### デバイス検出情報の取得
```json
// ログから実際のデバイス情報を確認
LOG_INFO("Detected device: {}", deviceName);
LOG_INFO("Instance name: {}", instanceName);  
LOG_INFO("GUID: {}", deviceGuid);

// この情報を device_info に設定
```

---

## 🚀 クイックスタート

### 1. デバイス確認
```bash
# GamepadMapperを起動してログ確認
./GamepadMapper.exe
# ログファイルで接続されたデバイス情報を確認
```

### 2. 設定ファイル選択
```bash
# デバイスに応じた設定ファイルをコピー
cp gamepad_config_xbox.json gamepad_config_mydevice.json
```

### 3. カスタマイズ
```json
// gamepad_config_mydevice.json を編集
{
  "device_info": {
    "name": "実際のデバイス名",  // ログで確認した値
    "instance_name": "実際のインスタンス名",
    "guid": "実際のGUID"
  },
  "gamepad": {
    // 好みに応じてキー配置を変更
  }
}
```

### 4. テスト・調整
```bash
# 再起動して動作確認
./GamepadMapper.exe
# 必要に応じて感度調整
```

---

## 🔍 トラブルシューティング

### よくある問題

#### 設定ファイルが読み込まれない
- ✅ ファイル名がデバイス名と一致しているか確認
- ✅ JSON形式が正しいか確認（オンラインバリデータ使用）
- ✅ ファイルアクセス権限を確認

#### キーが反応しない
- ✅ キー名が有効な値か確認（上記リスト参照）
- ✅ 大文字小文字を確認（すべて小文字で記述）
- ✅ ログでボタンインデックスを確認

#### スティックの感度が悪い
- ✅ `stick_threshold` を調整（低=高感度）
- ✅ `deadzone_percentage` を調整（低=敏感）
- ✅ デバイス固有の特性を考慮

---

**詳細なトラブルシューティングは [TROUBLESHOOTING.md](TROUBLESHOOTING.md) を参照してください。**