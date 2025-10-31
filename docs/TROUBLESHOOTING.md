# GamepadMapper トラブルシューティング ガイド

## 🔧 問題解決ガイド

このガイドでは、GamepadMapperの使用中に遭遇する可能性のある問題と解決方法を説明します。

---

## 📋 目次
- [インストール・ビルド問題](#インストールビルド問題)
- [デバイス認識問題](#デバイス認識問題)
- [入力変換問題](#入力変換問題)
- [設定ファイル問題](#設定ファイル問題)
- [パフォーマンス問題](#パフォーマンス問題)
- [ログ・デバッグ](#ログデバッグ)
- [よくある質問](#よくある質問)

---

## 🏗️ インストール・ビルド問題

### ❌ ビルドエラー: vcpkg依存関係が見つからない

#### 症状
```bash
CMake Error: Could not find spdlog
CMake Error: Could not find nlohmann-json
```

#### 解決方法
```bash
# 1. vcpkgを最新に更新
cd vcpkg
git pull
./bootstrap-vcpkg.sh  # Linux/macOS
./bootstrap-vcpkg.bat  # Windows

# 2. 依存関係を再インストール
vcpkg install spdlog:x64-mingw-dynamic
vcpkg install nlohmann-json:x64-mingw-dynamic
vcpkg install fmt:x64-mingw-dynamic

# 3. vcpkg統合を再実行
vcpkg integrate install

# 4. CMakeキャッシュをクリア
rm -rf build/
bash build.sh
```

### ❌ コンパイルエラー: C++20機能が使用できない

#### 症状
```bash
error: 'std::format' is not a member of 'std'
error: 'concept' was not declared in this scope
```

#### 解決方法
```bash
# GCCバージョン確認
gcc --version  # 9.0以上必要

# MinGW-w64の場合
pacman -S mingw-w64-x86_64-gcc  # 最新版インストール

# CMakeでC++20明示的指定
cmake -DCMAKE_CXX_STANDARD=20 -B build
```

### ❌ リンクエラー: DirectInput8ライブラリが見つからない

#### 症状
```bash
undefined reference to `DirectInput8Create'
```

#### 解決方法
```cmake
# CMakeLists.txtでライブラリリンク確認
target_link_libraries(GamepadMapper PRIVATE
    dinput8
    dxguid
    user32
    # その他
)
```

---

## 🎮 デバイス認識問題

### ❌ ゲームパッドが検出されない

#### 症状
- アプリ起動時にデバイス数が0と表示
- ログに「No gamepad devices found」

#### 診断手順
```bash
# 1. Windowsデバイスマネージャーで確認
# コントロールパネル > デバイスマネージャー > ヒューマンインターフェースデバイス

# 2. ログファイル確認
tail -f gamepad_mapper.log

# 3. デバッグモードで起動
GamepadMapper.exe --debug
```

#### 解決方法

**パターンA: デバイスドライバ問題**
```bash
# 1. デバイスマネージャーでドライバ更新
# 2. Xbox Game Bar無効化（競合する場合）
# 3. Steam Input無効化（Steamコントローラー使用時）
```

**パターンB: アクセス権限問題**
```bash
# 管理者権限で実行
# または
# ユーザーアカウント制御(UAC)を調整
```

**パターンC: DirectInput初期化失敗**
```cpp
// ログで以下を確認
LOG_ERROR("DirectInput8Create failed. HRESULT: 0x80070005");
// → アクセス拒否エラー

LOG_ERROR("DirectInput8Create failed. HRESULT: 0x80040154");
// → COMコンポーネント未登録
```

### ❌ デバイスが途中で切断される

#### 症状
```bash
LOG_WARN("Device lost or not acquired: Xbox Controller. Trying to reconnect.");
```

#### 解決方法
```bash
# 1. USBポート変更
# 2. バッテリー残量確認（ワイヤレスの場合）
# 3. 他のアプリケーションとの競合確認
# 4. 再接続間隔調整（設定ファイル）
{
  "config": {
    "reconnect_interval_ms": 5000,
    "max_reconnect_attempts": 10
  }
}
```

### ❌ 複数デバイスで一部だけ認識されない

#### 症状
- デバイス1は動作するがデバイス2が無反応
- ログに特定デバイスのエラーが頻出

#### 診断・解決
```bash
# 1. デバイス固有の設定ファイル確認
ls gamepad_config_*.json

# 2. GUID重複チェック
grep -r "guid" gamepad_config_*.json

# 3. デバイス名衝突確認
# 同じ製品名の場合、インスタンス名で区別

# 4. 個別に接続テスト
# 1台ずつ接続して動作確認
```

---

## ⌨️ 入力変換問題

### ❌ ボタンを押してもキー入力が発生しない

#### 症状
- デバイス検出はされるが、入力が反映されない
- ログに「Button0 -> Keys[] PRESSED」（空のキー配列）

#### 解決方法
```json
// 1. 設定ファイル確認
{
  "gamepad": {
    "buttons": [
      {"index": 0, "keys": ["z"]},  // ← キー設定が正しいか
      {"index": 1, "keys": []}      // ← 空配列は無効
    ]
  }
}

// 2. キー名の妥当性確認
// 無効なキー名例: "button_a", "key_z", "VK_A"
// 正しいキー名例: "a", "z", "ctrl", "f1"
```

#### デバッグログ有効化
```cpp
// ログレベルをDEBUGに変更
LOG_DEBUG_W(L"Button0 -> Keys[0x5A] PRESSED (Config: config.json)");
// 0x5A = Z キーが正しく変換されているか確認
```

### ❌ 間違ったキーが入力される

#### 症状
- Aボタンを押したのにBキーが入力される
- 設定と異なるキーが送信される

#### 解決方法
```bash
# 1. ボタンインデックス確認
# ログで実際に押されたボタン番号を確認
LOG_DEBUG("Button index: 2, Expected: 0")

# 2. 設定ファイルとデバイスの対応確認
# デバイス固有の設定ファイルが正しいか

# 3. キー変換テーブル確認
# KeyResolver::ResolveKey() の動作確認
```

### ❌ アナログスティックが反応しない

#### 症状
- スティック操作が認識されない
- 閾値設定が適切でない

#### 解決方法
```json
{
  "config": {
    "stick_threshold": 400  // デフォルト値を調整
  }
}

// 推奨値:
// - 敏感にしたい場合: 200-300
// - 通常: 400-500  
// - 鈍感にしたい場合: 600-800
```

#### スティック値の確認
```cpp
// デバッグログでスティック値確認
LOG_DEBUG("Stick X: {}, Y: {}, Threshold: {}", js.lX, js.lY, threshold);
// 値が閾値を超えているか確認
```

---

## ⚙️ 設定ファイル問題

### ❌ 設定ファイルが読み込まれない

#### 症状
```bash
LOG_ERROR("Failed to load configuration for device: Xbox Controller");
LOG_DEBUG("Config file exists: NO");
```

#### 解決方法
```bash
# 1. ファイルパス確認
LOG_DEBUG("Config file path: C:\path\to\gamepad_config_Xbox_Controller.json");

# 2. ファイルアクセス権限確認
# 読み取り権限があるか

# 3. JSON形式確認
# オンラインJSONバリデータで検証

# 4. 手動でファイル作成
{
  "device_info": {
    "name": "Xbox Controller",
    "instance_name": "Controller (Xbox One For Windows)",
    "guid": "{12345678-1234-1234-1234-123456789ABC}"
  },
  "gamepad": {
    "buttons": [
      {"index": 0, "keys": ["z"]}
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

### ❌ 設定変更が反映されない

#### 症状
- 設定ファイルを編集したが、動作が変わらない
- 古い設定のまま動作する

#### 解決方法
```bash
# 1. アプリケーション再起動
# 設定はアプリ起動時に読み込まれる

# 2. 設定ファイル保存確認
# エディタで確実に保存されているか

# 3. ファイル名確認
# デバイス名に基づく正しいファイル名か
# gamepad_config_{デバイス名}.json

# 4. JSON構文エラーチェック
# 無効なJSONの場合、読み込みがスキップされる
```

### ❌ 特殊文字を含むデバイス名でファイル作成できない

#### 症状
```bash
LOG_ERROR("Failed to create config file: Invalid filename characters");
```

#### 解決方法
```cpp
// 自動的に安全なファイル名に変換される
// 例: "Controller (Special/Edition)" → "Controller_Special_Edition"

// 手動で確認する場合
std::string safeDeviceName = GetSafeFileName();
LOG_DEBUG("Safe filename: {}", safeDeviceName);
```

---

## ⚡ パフォーマンス問題

### ❌ CPU使用率が高い

#### 症状
- アプリケーションのCPU使用率が20%以上
- システム全体が重くなる

#### 診断
```bash
# 1. タスクマネージャーでCPU使用率確認
# 2. プロファイラでホットスポット特定

# 3. ログレベル調整
{
  "config": {
    "log_level": "warn"  // debugからwarnに変更
  }
}
```

#### 解決方法
```bash
# 1. ポーリング頻度調整
# メインループのSleep時間調整

# 2. デバイススキャン間隔調整
# デフォルト5秒間隔を10秒に延長

# 3. フレームログ削減
# 高頻度ログの出力を制限
```

### ❌ メモリリーク

#### 症状
- 長時間使用でメモリ使用量が増加し続ける
- システムが不安定になる

#### 診断
```bash
# Visual Studio Diagnostic Tools使用
# または Application Verifier

# メモリ使用量モニタリング
while true; do
    ps aux | grep GamepadMapper | awk '{print $6}'
    sleep 60
done
```

#### 解決方法
```cpp
// 1. COMオブジェクトの適切な解放確認
device.Reset();  // Microsoft::WRL::ComPtr

// 2. RAII原則の遵守
{
    auto device = std::make_unique<GamepadDevice>();
    // 自動的にデストラクタで解放
}

// 3. 循環参照の回避
// shared_ptr使用時に注意
```

### ❌ 入力遅延

#### 症状
- ボタンを押してからキー入力まで遅延がある
- ゲームプレイに支障がでる

#### 診断・解決
```bash
# 1. ポーリング頻度確認
# 1ms間隔が理想（現在の設定確認）

# 2. ログ出力削減
# デバッグログが大量出力されていないか

# 3. 他アプリケーション競合確認
# Steam Input, Xbox Game Bar等の無効化

# 4. プロセス優先度調整
# タスクマネージャーで「高」に設定
```

---

## 📊 ログ・デバッグ

### 📋 ログレベル設定

#### ログレベル一覧
```json
{
  "config": {
    "log_level": "trace"   // 最も詳細
    "log_level": "debug"   // デバッグ情報
    "log_level": "info"    // 一般情報（デフォルト）
    "log_level": "warn"    // 警告のみ
    "log_level": "error"   // エラーのみ
    "log_level": "critical" // クリティカルエラーのみ
  }
}
```

#### 用途別推奨設定
```bash
# 通常使用
"log_level": "info"

# 問題調査
"log_level": "debug"

# パフォーマンス重視
"log_level": "warn"

# 開発時
"log_level": "trace"
```

### 🔍 重要なログメッセージ

#### デバイス関連
```bash
# 正常
LOG_INFO("New gamepad device added: Xbox Controller (Controller (Xbox One For Windows))")
LOG_INFO("Device reconnected successfully: Xbox Controller")

# 警告
LOG_WARN("Device lost or not acquired: Xbox Controller. Trying to reconnect.")
LOG_WARN("Initial device acquisition failed for: Xbox Controller (may work in background)")

# エラー
LOG_ERROR("Failed to create device: Xbox Controller. HRESULT: 0x80070005")
LOG_ERROR("SetDataFormat failed. HRESULT: 0x80040154")
```

#### 設定関連
```bash
# 正常
LOG_INFO("Configuration loaded for device: Xbox Controller (file: config.json)")
LOG_INFO("Default configuration created successfully for device: Xbox Controller")

# エラー
LOG_ERROR("Failed to load configuration for device: Xbox Controller")
LOG_ERROR("Failed to create new configuration for device: Xbox Controller")
```

#### 入力関連
```bash
# デバッグ情報
LOG_DEBUG("Button0 -> Keys[0x5A] PRESSED (Config: config.json)")
LOG_DEBUG("POV Up -> Keys[0x26] ON")
LOG_DEBUG("Axis Left -> Keys[0x41] ON")
```

### 🛠️ デバッグモード

#### コマンドライン引数
```bash
# デバッグモードで起動
GamepadMapper.exe --debug

# ログレベル指定
GamepadMapper.exe --log-level=trace

# コンソール出力有効
GamepadMapper.exe --console
```

#### デバッグ情報の確認
```bash
# ログファイル監視
tail -f gamepad_mapper.log

# 特定の問題を絞り込み
grep "ERROR" gamepad_mapper.log
grep "Device" gamepad_mapper.log | grep "failed"
```

---

## ❓ よくある質問

### Q1: 対応しているゲームパッドは？

**A**: DirectInput対応のゲームパッドであれば基本的に使用可能です。

**確認済みデバイス**:
- Xbox One/Series X/S Controller
- PlayStation 4/5 Controller (DS4Windows不要)
- Nintendo Switch Pro Controller
- 汎用USB/Bluetooth ゲームパッド

**非対応**:
- XInput専用ゲームパッド（DirectInput非対応）
- キーボード・マウス

### Q2: 設定ファイルの場所は？

**A**: 実行ファイルと同じディレクトリに自動作成されます。

```bash
GamepadMapper.exe
gamepad_config_Xbox_Controller.json      # デバイス別設定
gamepad_config_PS4_Controller.json
gamepad_mapper.log                       # ログファイル
```

### Q3: 複数のゲームパッドで同じキーに割り当てできる？

**A**: はい、可能です。各デバイスは独立して動作します。

```json
// デバイス1の設定
{"index": 0, "keys": ["z"]}

// デバイス2の設定（同じキーでもOK）
{"index": 0, "keys": ["z"]}
```

### Q4: ゲーム中に設定を変更できる？

**A**: 現在は再起動が必要です。リアルタイム設定変更は将来のバージョンで対応予定です。

### Q5: 他のゲームパッド関連ソフトと競合する？

**A**: 以下のソフトウェアと競合する可能性があります：

**競合する可能性あり**:
- Steam Input（Steam実行時）
- Xbox Game Bar
- DS4Windows
- Xpadder, JoyToKey等

**解決方法**:
- 他のソフトを無効化
- 使用するソフトを1つに限定

### Q6: Linux/macOS対応予定は？

**A**: 現在Windows専用ですが、クロスプラットフォーム対応を検討中です。

### Q7: 商用利用は可能？

**A**: ライセンス条項に従って利用可能です。詳細はLICENSEファイルを確認してください。

---

## 🆘 サポート・報告

### 🐛 バグ報告

GitHub Issuesで以下の情報を提供してください：

```markdown
## 環境情報
- OS: Windows 10/11 (ビルド番号)
- GamepadMapper Version: v1.x.x
- デバイス: Xbox One Controller
- 接続方法: USB/Bluetooth

## 再現手順
1. アプリケーション起動
2. デバイス接続
3. 特定のボタンを押す
4. エラーが発生

## 期待される動作
Zキーが入力されるはず

## 実際の動作  
何も入力されない

## ログファイル
```
LOG_ERROR("Button mapping failed for device: Xbox Controller")
```

## 追加情報
設定ファイル、スクリーンショット等
```

### 💬 質問・相談

- **GitHub Discussions**: 一般的な質問
- **GitHub Issues**: バグ報告、機能要求

### 📧 緊急連絡

クリティカルなセキュリティ問題の場合は、公開のIssueではなく直接連絡してください。

---

## 🔧 セルフチェック リスト

問題が発生した場合、以下を順次確認してください：

### 基本確認
- [ ] ゲームパッドがWindowsで認識されている
- [ ] アプリケーションが管理者権限で実行されている
- [ ] 他のゲームパッド関連ソフトが無効化されている
- [ ] 最新バージョンを使用している

### ログ確認
- [ ] `gamepad_mapper.log` ファイルが存在する
- [ ] ログにエラーメッセージが出力されていない
- [ ] デバイス検出ログが正常に出力されている

### 設定確認
- [ ] 設定ファイルが存在する
- [ ] JSON形式が正しい
- [ ] キー名が有効である
- [ ] ボタンインデックスが正しい

### パフォーマンス確認
- [ ] CPU使用率が正常範囲内（<10%）
- [ ] メモリ使用量が増加し続けていない
- [ ] 入力遅延が許容範囲内（<50ms）

---

**問題が解決しない場合は、遠慮なくサポートにお問い合わせください！**

**最終更新**: 2024年  
**サポート担当**: Rovo Dev