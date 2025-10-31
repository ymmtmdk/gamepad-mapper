# GamepadMapper 設定ファイル

このフォルダには、GamepadMapperの設定ファイルとサンプルが含まれています。

## 📁 フォルダ構成

```
config/
├── README.md                    # このファイル
├── samples/                     # サンプル設定ファイル
│   ├── gamepad_config_xbox.json           # Xbox Controller標準設定
│   ├── gamepad_config_ps4.json            # PS4 Controller設定
│   ├── gamepad_config_switch_pro.json     # Switch Pro Controller設定
│   ├── gamepad_config_gaming.json         # ゲーム用最適化設定
│   ├── gamepad_config_productivity.json   # 作業効率化設定
│   └── gamepad_config_media.json          # メディア再生用設定
│
└── [user_configs]/              # ユーザー作成の設定ファイル
    └── gamepad_config_*.json    # 実際に使用する設定ファイル
```

## 🚀 クイックスタート

### 1. サンプルファイルをコピー
```bash
# 適切なサンプルファイルをコピー
cp config/samples/gamepad_config_xbox.json config/gamepad_config_mydevice.json
```

### 2. デバイス情報を更新
```bash
# GamepadMapperを起動してデバイス情報を確認
./GamepadMapper.exe
# ログファイルでデバイス名・GUIDを確認し、設定ファイルを編集
```

### 3. カスタマイズ
```json
{
  "device_info": {
    "name": "実際のデバイス名",      // ログで確認した値
    "instance_name": "実際のインスタンス名",
    "guid": "実際のGUID"
  },
  // 好みに応じてキー配置を変更
}
```

## 🎮 サンプルファイル詳細

### デバイス別設定

#### `gamepad_config_xbox.json`
- **対象**: Xbox One/Series Controller
- **特徴**: 標準的な配置、汎用性重視
- **推奨用途**: 一般的な用途、初回設定

#### `gamepad_config_ps4.json`
- **対象**: PlayStation 4 Controller
- **特徴**: PlayStation固有の配置、低デッドゾーン
- **推奨用途**: PSユーザー、精密操作重視

#### `gamepad_config_switch_pro.json`
- **対象**: Nintendo Switch Pro Controller
- **特徴**: Nintendo配置、14ボタン対応
- **推奨用途**: Switchユーザー、多ボタン活用

### 用途別設定

#### `gamepad_config_gaming.json`
- **最適化**: 高速応答（閾値300）、低デッドゾーン（5%）
- **推奨用途**: アクションゲーム、FPS、格闘ゲーム
- **特徴**: パフォーマンス最優先、振動有効

#### `gamepad_config_productivity.json`
- **最適化**: 精密操作（閾値500）、Ctrl+キー組み合わせ
- **推奨用途**: オフィス作業、プログラミング、ブラウジング
- **特徴**: 誤操作防止、静音（振動無効）

#### `gamepad_config_media.json`
- **最適化**: メディア再生操作、シーク・音量制御
- **推奨用途**: YouTube、Netflix、音楽プレーヤー
- **特徴**: 再生制御特化、フルスクリーン対応

## ⚙️ 設定ファイル作成方法

### 1. **デバイス情報の取得**
```bash
# GamepadMapperを起動
./GamepadMapper.exe

# ログファイルで以下を確認
LOG_INFO("Device detected: {}", deviceName);
LOG_INFO("Instance name: {}", instanceName);  
LOG_INFO("GUID: {}", deviceGuid);
```

### 2. **適切なサンプル選択**
```bash
# デバイスタイプで選択
Xbox系 → gamepad_config_xbox.json
PlayStation系 → gamepad_config_ps4.json
Switch Pro → gamepad_config_switch_pro.json

# 用途で選択
ゲーム → gamepad_config_gaming.json
仕事 → gamepad_config_productivity.json
動画・音楽 → gamepad_config_media.json
```

### 3. **設定ファイル配置**
```bash
# プロジェクトルートに配置（自動検出）
cp config/samples/選択したファイル.json gamepad_config_実際のデバイス名.json

# または config/ フォルダ内に配置
cp config/samples/選択したファイル.json config/gamepad_config_実際のデバイス名.json
```

## 🔧 カスタマイズ例

### ボタン配置の変更
```json
{
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["space"] },        // Aボタン → スペース
      { "index": 1, "keys": ["ctrl", "c"] },    // Bボタン → Ctrl+C
      { "index": 2, "keys": ["alt", "f4"] }     // Xボタン → Alt+F4
    ]
  }
}
```

### 感度調整
```json
{
  "config": {
    "stick_threshold": 300,        // 300-600（低=高感度）
    "deadzone_percentage": 8,      // 5-15（低=敏感）
    "enable_vibration": false,     // 振動の有効/無効
    "log_level": "debug"          // ログレベル調整
  }
}
```

### 複雑なキー組み合わせ
```json
{
  "gamepad": {
    "buttons": [
      { "index": 8, "keys": ["ctrl", "alt", "delete"] },  // 3キー同時
      { "index": 9, "keys": ["win", "l"] }                 // Windowsロック
    ]
  }
}
```

## 🔍 トラブルシューティング

### 設定ファイルが読み込まれない
1. **ファイル名確認**: `gamepad_config_{device_name}.json`
2. **配置場所確認**: プロジェクトルートまたは`config/`フォルダ
3. **JSON形式確認**: オンラインJSONバリデータで検証
4. **デバイス名確認**: ログで実際のデバイス名を確認

### キーが反応しない
1. **キー名確認**: 有効なキー名を使用（[詳細ガイド](../docs/CONFIGURATION_GUIDE.md)参照）
2. **ボタンインデックス確認**: ログでボタン番号を確認
3. **大文字小文字**: すべて小文字で記述

### スティック感度が悪い
1. **閾値調整**: `stick_threshold`を変更（低=高感度）
2. **デッドゾーン調整**: `deadzone_percentage`を変更（低=敏感）
3. **ログ確認**: DEBUG レベルでスティック値を確認

## 📚 詳細情報

- **[設定ガイド](../docs/CONFIGURATION_GUIDE.md)** - 詳細な設定方法・カスタマイズ
- **[トラブルシューティング](../docs/TROUBLESHOOTING.md)** - よくある問題と解決法
- **[開発者ガイド](../docs/DEVELOPER_GUIDE.md)** - 開発環境・実装情報

---

**設定ファイルの作成・カスタマイズで困った場合は、上記の詳細ガイドを参照するか、GitHubのIssuesでサポートを受けてください。**