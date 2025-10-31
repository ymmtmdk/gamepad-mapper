# GamepadMapper トラブルシューティング（簡潔版）

## 🔧 よくある問題と解決法

### 🏗️ ビルド問題

#### vcpkg依存関係エラー
```bash
# エラー: Could not find spdlog
vcpkg install spdlog:x64-mingw-dynamic nlohmann-json:x64-mingw-dynamic fmt:x64-mingw-dynamic
vcpkg integrate install
rm -rf build/ && bash build.sh
```

#### MinGW/GCCバージョン問題
```bash
# GCC 9.0+ が必要
gcc --version
# 古い場合は更新または Visual Studio使用
```

---

### 🎮 デバイス認識問題

#### デバイスが検出されない
```bash
# 1. Windowsでデバイス確認
# 設定 > デバイス > Bluetooth とその他のデバイス

# 2. 管理者権限で実行
# 右クリック → 管理者として実行

# 3. 他のゲームパッドソフト無効化
# Steam Big Picture、Xpadder等を終了
```

#### 接続が不安定
```bash
# USB接続推奨（Bluetoothより安定）
# ログでデバイス状態確認
LOG_DEBUG("Device status: Connected={}, Acquired={}", connected, acquired);
```

---

### ⌨️ 入力変換問題

#### キーが反応しない
```json
// 設定ファイル確認 (gamepad_config_*.json)
{
  "gamepad": {
    "buttons": [
      { "index": 0, "keys": ["z"] }  // 有効なキー名使用
    ]
  }
}

// 無効なキー名例: "button_a", "VK_A", "key_z"
// 有効なキー名例: "a", "z", "ctrl", "f1", "enter"
```

#### 間違ったキーが入力される
```cpp
// ボタンインデックス確認
LOG_DEBUG("Button index: {} pressed", buttonIndex);
// 設定ファイルとの対応確認
```

#### アナログスティック反応しない
```json
{
  "config": {
    "stick_threshold": 400  // 値を調整
  }
}
// 敏感: 200-300, 通常: 400-500, 鈍感: 600-800
```

---

### ⚙️ 設定ファイル問題

#### 設定が読み込まれない
```bash
# 1. ファイル存在確認
ls gamepad_config_*.json

# 2. JSON形式確認
# オンラインJSONバリデータで検証

# 3. ファイルアクセス権限確認
# 読み書き権限があるか確認
```

#### デバイス名が合わない
```cpp
// ログでデバイス名確認
LOG_INFO_W(L"Device detected: {}", deviceName);
// 設定ファイル名を実際のデバイス名に合わせる
```

---

### 📊 パフォーマンス問題

#### CPU使用率が高い
```cpp
// メインループ内の処理時間測定
auto start = std::chrono::high_resolution_clock::now();
ProcessInput();
auto end = std::chrono::high_resolution_clock::now();
LOG_DEBUG("Process time: {}ms", duration_cast<milliseconds>(end-start).count());
```

#### メモリリーク
```cpp
// スマートポインタ使用確認
std::unique_ptr<GamepadDevice> device;
std::shared_ptr<IDisplayBuffer> buffer;
```

---

### 📝 ログ・デバッグ

#### ログが出力されない
```cpp
// ログレベル確認・変更
LOG_SET_LEVEL(spdlog::level::debug);
LOG_DEBUG("Debug message test");
```

#### ログファイル確認
```bash
# ログファイル場所
./gamepad_mapper.log        # メインログ
./gamepad_config_*.json     # 設定ファイル

# ログ内容例
LOG_INFO("GamepadMapper starting...")
LOG_ERROR("Device initialization failed")
```

---

## 🆘 緊急時チェックリスト

問題発生時は以下を順次確認：

### 基本確認（1分）
- [ ] ゲームパッドがWindowsで認識されている
- [ ] 管理者権限で実行している
- [ ] 他のゲームパッド関連ソフトが停止している

### ログ確認（2分）
- [ ] `gamepad_mapper.log` が存在する
- [ ] エラーメッセージがない
- [ ] デバイス検出ログが正常

### 設定確認（2分）
- [ ] `gamepad_config_*.json` が存在する
- [ ] JSON形式が正しい
- [ ] キー名が有効

---

## 🐛 バグ報告テンプレート

```markdown
## 環境
- OS: Windows 10/11
- GamepadMapper: v1.x.x
- デバイス: Xbox Controller/PS4 Controller
- 接続: USB/Bluetooth

## 現象
[具体的な問題の説明]

## 再現手順
1. アプリ起動
2. [具体的な操作]
3. エラー発生

## ログ
```
[gamepad_mapper.logの関連部分]
```

## 設定ファイル
[gamepad_config_*.json の内容]
```

---

**解決しない場合は GitHub Issues でサポートを受けてください！**