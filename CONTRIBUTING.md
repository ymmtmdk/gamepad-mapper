# GamepadMapper コントリビューション ガイド

## 🤝 プロジェクトへの貢献方法

GamepadMapperプロジェクトへの貢献を歓迎します！このガイドは、効果的で一貫性のある貢献を行うためのガイドラインです。

---

## 📋 目次
- [行動規範](#行動規範)
- [貢献の種類](#貢献の種類)
- [開発環境セットアップ](#開発環境セットアップ)
- [Pull Request プロセス](#pull-request-プロセス)
- [コーディング規約](#コーディング規約)
- [テスト規約](#テスト規約)
- [ドキュメント規約](#ドキュメント規約)

---

## 🤝 行動規範

### 基本原則
- **尊重**: 全ての貢献者とユーザーを尊重する
- **協力**: 建設的なフィードバックと協力的な議論
- **品質**: 高品質なコードとドキュメントの維持
- **学習**: お互いの学習と成長を支援

### 禁止事項
- 攻撃的、差別的、ハラスメント的な言動
- スパム、商業的宣伝
- 他者の知的財産権の侵害

---

## 🎯 貢献の種類

### 1. **バグ報告**
新しいバグを発見した場合：

#### 🐛 バグ報告テンプレート
```markdown
## バグ説明
簡潔で明確なバグの説明

## 再現手順
1. ...
2. ...
3. ...

## 期待される動作
何が起きるべきか

## 実際の動作
何が実際に起きたか

## 環境情報
- OS: Windows 10/11
- コンパイラ: MinGW-w64 / MSVC
- バージョン: v1.x.x
- デバイス: Xbox Controller / PS4 Controller

## 追加情報
ログファイル、スクリーンショット、設定ファイル等
```

### 2. **機能要求**
新機能の提案：

#### ✨ 機能要求テンプレート
```markdown
## 機能概要
実装したい機能の簡潔な説明

## 動機・理由
なぜこの機能が必要か

## 詳細な仕様
- 具体的な動作
- UI/UXの考慮点
- 設定ファイル変更が必要か

## 代替案
検討した他の解決方法

## 実装の複雑度
- [ ] 簡単（1-2時間）
- [ ] 中程度（1-2日）
- [ ] 複雑（1週間以上）
```

### 3. **コード貢献**
以下の領域での貢献を歓迎：

- **新デバイスサポート**: 新しいゲームパッドブランドの対応
- **入力機能拡張**: マクロ、シーケンシャル入力等
- **UI改善**: 設定GUI、リアルタイム表示改良
- **パフォーマンス最適化**: メモリ使用量、CPU負荷削減
- **クロスプラットフォーム**: Linux、macOS対応
- **テスト追加**: 単体テスト、統合テスト

### 4. **ドキュメント改善**
- README、開発者ガイド更新
- APIドキュメント拡充
- チュートリアル作成
- 翻訳（英語⇔日本語）

---

## 🛠️ 開発環境セットアップ

### 1. **前提条件**
```bash
# 必要ツール
- Git 2.30+
- CMake 3.20+
- vcpkg
- MinGW-w64 または Visual Studio 2019+
```

### 2. **リポジトリセットアップ**
```bash
# 1. フォーク＆クローン
git clone https://github.com/YOUR_USERNAME/GamepadMapper.git
cd GamepadMapper

# 2. 上流リポジトリ追加
git remote add upstream https://github.com/ORIGINAL_OWNER/GamepadMapper.git

# 3. 依存関係インストール
vcpkg install spdlog:x64-mingw-dynamic nlohmann-json:x64-mingw-dynamic fmt:x64-mingw-dynamic

# 4. ビルド確認
bash build.sh
```

### 3. **開発ブランチ作成**
```bash
# 最新の変更を取得
git fetch upstream
git checkout main
git merge upstream/main

# 機能ブランチ作成
git checkout -b feature/xbox-series-x-support
# または
git checkout -b bugfix/device-reconnection-crash
# または  
git checkout -b docs/api-reference-update
```

---

## 📝 Pull Request プロセス

### 1. **開発フロー**

#### 🔄 標準的な開発サイクル
```bash
# 1. 機能実装
# ... コード変更 ...

# 2. ビルド確認
bash build.sh

# 3. コミット
git add .
git commit -m "feat: Add Xbox Series X controller support

- Implement device detection for new Xbox Series X GUID
- Add default button mappings in JsonConfigManager  
- Update device enumeration logic in MultipleGamepadManager
- Add unit tests for new device type

Fixes #123"

# 4. プッシュ
git push origin feature/xbox-series-x-support

# 5. Pull Request作成
```

### 2. **コミットメッセージ規約**

#### 📋 Conventional Commits形式
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

#### タイプ一覧
- `feat`: 新機能
- `fix`: バグ修正  
- `docs`: ドキュメント変更
- `style`: フォーマット変更（機能に影響なし）
- `refactor`: リファクタリング
- `perf`: パフォーマンス改善
- `test`: テスト追加・修正
- `build`: ビルドシステム変更
- `ci`: CI設定変更
- `chore`: その他のタスク

#### 例
```bash
# 良い例
git commit -m "feat(input): Add macro support for button combinations

Implement macro recording and playback functionality:
- Add MacroRecorder class for sequence capture
- Extend InputProcessor to handle macro playback
- Update configuration format to include macro definitions

Closes #45"

git commit -m "fix(device): Resolve memory leak in device reconnection

The reconnection logic was not properly releasing DirectInput 
device objects, causing memory accumulation over time.

- Call Release() on old device before creating new one
- Add RAII wrapper for DirectInput device management
- Update reconnection tests

Fixes #67"

# 悪い例
git commit -m "fix stuff"
git commit -m "update"
git commit -m "追加"
```

### 3. **Pull Request チェックリスト**

#### ✅ PR作成前確認
- [ ] **ビルド成功**: `bash build.sh` がエラーなく完了
- [ ] **テスト通過**: 既存テストが全て成功
- [ ] **コーディング規約準拠**: スタイルガイドに従っている
- [ ] **ドキュメント更新**: 必要に応じてREADME、API文書更新
- [ ] **ログメッセージ**: 適切なログレベルと内容
- [ ] **設定ファイル**: 後方互換性の確保
- [ ] **メモリリーク**: 長時間実行でのメモリ確認

#### 📝 PR テンプレート
```markdown
## 変更概要
この PR の目的と実装内容の簡潔な説明

## 変更タイプ
- [ ] 🐛 バグ修正
- [ ] ✨ 新機能
- [ ] 📝 ドキュメント
- [ ] 🎨 スタイル修正
- [ ] ♻️ リファクタリング
- [ ] ⚡ パフォーマンス改善
- [ ] ✅ テスト追加

## 関連Issue
Closes #123
Related to #456

## 変更内容
### 追加
- Xbox Series X コントローラーサポート
- 新しいデバイス検出ロジック

### 変更
- デバイス列挙処理の最適化
- エラーハンドリングの改善

### 削除
- 使用されていないヘルパー関数

## テスト内容
- [ ] 新デバイスでの基本動作確認
- [ ] 既存デバイスへの影響確認
- [ ] メモリリークテスト実施
- [ ] 長時間動作テスト実施

## スクリーンショット・ログ
必要に応じて動作確認のスクリーンショットやログファイルを添付

## レビュー観点
特に重点的にレビューしてほしい箇所があれば記載

## 破壊的変更
- [ ] 破壊的変更はありません
- [ ] 破壊的変更があります（下記に詳細）

### 破壊的変更の詳細
設定ファイル形式の変更、API変更等の詳細
```

---

## 🎨 コーディング規約

### 1. **命名規則**

#### クラス・構造体・列挙型
```cpp
// ✅ PascalCase
class GamepadDevice;
struct DeviceConfiguration;
enum class LogLevel;

// ❌ 避ける
class gamepad_device;
class gamepadDevice;
```

#### 関数・変数
```cpp
// ✅ camelCase
void processInput();
int connectedDeviceCount;
bool isDeviceReady();

// ✅ メンバ変数: m_プレフィックス
class InputProcessor {
private:
    std::vector<WORD> m_buttonMapping;
    bool m_isInitialized;
};

// ✅ 定数: k_プレフィックス or UPPER_CASE
const int k_MaxDevices = 16;
const std::string k_DefaultConfigFile = "config.json";
#define MAX_BUTTONS 128
```

#### ファイル名
```cpp
// ✅ PascalCase
GamepadDevice.h
InputProcessor.cpp
JsonConfigManager.h

// ❌ 避ける  
gamepad_device.h
inputProcessor.cpp
json-config-manager.h
```

### 2. **コードスタイル**

#### インデント・括弧
```cpp
// ✅ 4スペース、Allman style
class GamepadDevice 
{
public:
    bool Initialize(IDirectInput8* directInput) 
    {
        if (!directInput) 
        {
            LOG_ERROR("DirectInput pointer is null");
            return false;
        }
        
        // 処理...
        return true;
    }
    
private:
    bool m_initialized = false;
};
```

#### インクルード順序
```cpp
// 1. 対応するヘッダー（.cppファイルの場合）
#include "GamepadDevice.h"

// 2. プロジェクト内ヘッダー
#include "ModernLogger.h"
#include "JsonConfigManager.h"

// 3. C++標準ライブラリ
#include <memory>
#include <string>
#include <vector>

// 4. サードパーティライブラリ
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

// 5. システム/プラットフォーム固有（最後）
#include <windows.h>
#include <dinput.h>
```

#### コメント規約
```cpp
/**
 * @brief ゲームパッドデバイスを初期化します
 * 
 * DirectInputデバイスの作成、設定ファイルの読み込み、
 * 入力プロセッサの初期化を行います。
 * 
 * @param pDirectInput DirectInput8インターフェース（非NULL）
 * @param pdidInstance デバイスインスタンス情報
 * @param hWnd ウィンドウハンドル
 * @return 成功時true、失敗時false
 * 
 * @note この関数は1つのデバイスにつき1回のみ呼び出し可能
 * @warning DirectInputが初期化されている必要があります
 * 
 * @see Shutdown()
 * @since v1.0.0
 */
bool Initialize(IDirectInput8* pDirectInput, 
               const DIDEVICEINSTANCE* pdidInstance, 
               HWND hWnd);

// 単行コメント：処理の意図説明（日本語OK）
// ボタン状態をチェックして前回と異なる場合のみ処理
for (size_t i = 0; i < MAX_BUTTONS; ++i) {
    bool pressed = (js.rgbButtons[i] & 0x80) != 0;
    if (pressed != m_prevButtons[i]) {
        ProcessButtonChange(i, pressed);
        m_prevButtons[i] = pressed; // 状態を保存
    }
}
```

### 3. **エラーハンドリング**

#### HRESULT チェック
```cpp
// ✅ 良い例
HRESULT hr = device->Acquire();
if (FAILED(hr)) {
    LOG_ERROR("Device acquisition failed. HRESULT: 0x{:08X}", hr);
    
    // 特定エラーに対する対処
    if (hr == DIERR_INPUTLOST) {
        return TryReconnect();
    }
    
    return false;
}
```

#### 例外処理
```cpp
// ✅ 良い例：RAII + 例外安全
try {
    auto config = std::make_unique<JsonConfigManager>(configPath);
    if (!config->load()) {
        throw std::runtime_error("Configuration loading failed");
    }
    
    // config使用...
    
} catch (const std::exception& e) {
    LOG_ERROR("Configuration error: {}", e.what());
    return false;
}
```

#### ポインタ チェック
```cpp
// ✅ 良い例：早期リターン
bool ProcessInput(const GamepadDevice* device) {
    if (!device) {
        LOG_ERROR("Device pointer is null");
        return false;
    }
    
    if (!device->IsConnected()) {
        LOG_WARN("Device is not connected");
        return false;
    }
    
    // 処理続行...
}
```

---

## 🧪 テスト規約

### 1. **テスト構成**

#### ディレクトリ構造
```
GamepadMapper/
├── src/           # 本体コード
├── tests/         # テストコード
│   ├── unit/      # 単体テスト
│   ├── integration/ # 統合テスト
│   └── fixtures/  # テストデータ
└── tools/         # テストツール
```

#### テストファイル命名
```cpp
// 単体テスト
test_InputProcessor.cpp      // InputProcessor クラスのテスト
test_JsonConfigManager.cpp   // JsonConfigManager クラスのテスト

// 統合テスト
integration_DeviceFlow.cpp       // デバイス全体フローテスト
integration_MultipleDevices.cpp  // 複数デバイステスト
```

### 2. **テスト実装**

#### 基本テストケース
```cpp
#include "InputProcessor.h"
#include <cassert>

void test_ButtonMapping() {
    // Arrange
    JsonConfigManager config("test_config.json");
    config.load();
    InputProcessor processor(config);
    
    // Act
    DIJOYSTATE2 testState = {};
    testState.rgbButtons[0] = 0x80; // Button 0 pressed
    processor.ProcessGamepadInput(testState);
    
    // Assert
    // 期待されるキーが送信されたかチェック
    // （実際のテストではモックやキャプチャ機能使用）
}

void test_ConfigurationLoading() {
    // Arrange
    std::string testConfig = R"({
        "gamepad": {
            "buttons": [
                {"index": 0, "keys": ["a"]}
            ]
        },
        "config": {
            "stick_threshold": 500
        }
    })";
    
    // テストファイル作成
    std::ofstream file("test_temp_config.json");
    file << testConfig;
    file.close();
    
    // Act
    JsonConfigManager config("test_temp_config.json");
    bool result = config.load();
    
    // Assert
    assert(result == true);
    assert(config.getStickThreshold() == 500);
    auto button0Keys = config.getButtonKeys(0);
    assert(button0Keys.size() == 1);
    assert(button0Keys[0] == VK_A);
    
    // Cleanup
    std::filesystem::remove("test_temp_config.json");
}
```

#### モック・スタブ使用
```cpp
// モックデバイス実装例
class MockDirectInputDevice : public IDirectInputDevice8 {
public:
    HRESULT GetDeviceState(DWORD cbData, LPVOID lpvData) override {
        if (m_simulateFailure) {
            return DIERR_INPUTLOST;
        }
        
        // テスト用状態を返す
        memcpy(lpvData, &m_testState, cbData);
        return S_OK;
    }
    
    void SetTestState(const DIJOYSTATE2& state) {
        m_testState = state;
    }
    
    void SimulateFailure(bool enable) {
        m_simulateFailure = enable;
    }
    
private:
    DIJOYSTATE2 m_testState = {};
    bool m_simulateFailure = false;
};
```

### 3. **テスト実行**

#### テストの実行方法
```bash
# 全テスト実行
bash run_tests.sh

# 特定テスト実行
bash run_tests.sh --filter="InputProcessor"

# メモリリークチェック付き
bash run_tests.sh --memory-check
```

---

## 📚 ドキュメント規約

### 1. **README更新**
機能追加時は以下を更新：
- 機能説明セクション
- 設定例
- 対応デバイス一覧
- ビルド要件（必要に応じて）

### 2. **APIドキュメント**
新しいパブリックメソッド追加時：
```cpp
/**
 * @brief [簡潔な説明]
 * @param [パラメータ名] [説明]
 * @return [戻り値の説明]
 * @throws [例外の説明]
 * @note [重要な注意事項]
 * @see [関連する関数・クラス]
 * @since [追加されたバージョン]
 */
```

### 3. **設定ファイル仕様書**
新しい設定項目追加時：
```markdown
#### `new_feature_setting`
- **型**: `boolean`
- **デフォルト**: `false`
- **説明**: 新機能を有効化するかどうか
- **例**: `"new_feature_setting": true`
- **バージョン**: v1.2.0 以降
```

---

## 🏆 レビュープロセス

### 1. **セルフレビュー**
PR作成前に以下を確認：
- [ ] コードの意図が明確
- [ ] パフォーマンスに問題なし
- [ ] セキュリティ脆弱性なし
- [ ] エラーハンドリング適切
- [ ] テストでカバーされている

### 2. **ピアレビュー**
レビュアーは以下を確認：
- [ ] 要件を満たしているか
- [ ] 設計が適切か
- [ ] コーディング規約準拠
- [ ] 十分なテストがあるか
- [ ] ドキュメントが更新されているか

### 3. **レビューコメント例**

#### 良いレビューコメント
```markdown
💡 **提案**: `std::vector<WORD>` を `std::span<const WORD>` に変更することで、不要なコピーを避けられるかもしれません。

🔍 **質問**: この関数で `nullptr` が渡される可能性はありますか？その場合のエラーハンドリングが必要かもしれません。

✅ **承認**: ログレベルの使い分けが適切で、デバッグ時に役立ちそうです。

📝 **改善**: この複雑な処理にコメントがあると理解しやすくなります。
```

#### 避けるべきレビューコメント
```markdown
❌ これはダメです。
❌ 書き直してください。
❌ 意味がわかりません。
```

---

## 🚀 リリースプロセス

### 1. **バージョニング**
[Semantic Versioning](https://semver.org/) に従う：
- `MAJOR.MINOR.PATCH` (例: `1.2.3`)
- **MAJOR**: 破壊的変更
- **MINOR**: 後方互換な機能追加
- **PATCH**: 後方互換なバグ修正

### 2. **リリースノート**
```markdown
# GamepadMapper v1.2.0

## 🎉 新機能
- Xbox Series X/S コントローラーサポート追加
- マクロ記録・再生機能実装
- リアルタイム設定変更UI

## 🐛 バグ修正
- デバイス再接続時のメモリリーク修正
- 長時間使用時のパフォーマンス問題解決

## ⚡ 改善
- 起動時間20%短縮
- メモリ使用量15%削減

## 💔 破壊的変更
- 設定ファイル形式の一部変更（自動マイグレーション対応）

## 📋 既知の問題
- 一部のBluetooth接続で初回認識に時間がかかる場合があります
```

---

## 🆘 サポート・質問

### 質問・相談窓口
- **GitHub Issues**: バグ報告、機能要求
- **GitHub Discussions**: 一般的な質問、アイデア議論
- **Discord**: リアルタイムな開発相談（もしあれば）

### 回答時間
- 通常：1-3営業日以内
- 緊急（クリティカルバグ）：24時間以内

---

**ありがとうございます！🎮**  
皆様の貢献により、GamepadMapperがより良いツールとなります。

**最終更新**: 2024年  
**維持管理者**: Rovo Dev