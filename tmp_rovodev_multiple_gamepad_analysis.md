# 複数ゲームパッド対応の実装分析と修正計画

## 現在の実装の問題点

### 1. 単一デバイス前提の設計
- **DirectInputManager**: 1つのデバイスのみ管理 (`m_pDevice`, `m_deviceConnected`)
- **Application**: 単一の `m_joystickState` と `m_directInputManager`
- **JsonConfigManager**: グローバルな設定ファイル1つのみ

### 2. 具体的な問題箇所

#### DirectInputManager.cpp
```cpp
// EnumDevicesCallback (line 233-253)
return DIENUM_STOP;  // 最初に見つけたデバイスで停止
```

#### Application.cpp  
```cpp
// ProcessGamepadInput (line 173-198)
// 単一のm_joystickStateとm_directInputManagerのみ処理
```

#### JsonConfigManager
- 設定ファイルが1つ固定 (`gamepad_config.json`)
- デバイス識別情報なし

## 修正すべき主要ポイント

### 1. アーキテクチャの変更

#### A. デバイス管理の多重化
- `DirectInputManager` → `MultipleGamepadManager`
- デバイスリスト管理: `std::vector<std::unique_ptr<GamepadDevice>>`
- デバイス識別: 名前、GUID、インスタンス名

#### B. 設定ファイルの個別化
- ファイル命名規則: `gamepad_config_{device_name}.json`
- デバイス固有設定の管理
- 設定ファイルの自動生成

#### C. 入力処理の並列化
- デバイスごとの `InputProcessor` インスタンス
- 同時入力の競合回避機能

### 2. 新しいクラス設計

#### GamepadDevice クラス
```cpp
class GamepadDevice {
private:
    ComPtr<IDirectInputDevice8> m_device;
    std::wstring m_name;
    std::wstring m_instanceName;
    GUID m_guid;
    bool m_connected;
    DIJOYSTATE2 m_currentState;
    std::unique_ptr<JsonConfigManager> m_config;
    std::unique_ptr<InputProcessor> m_processor;
};
```

#### MultipleGamepadManager クラス
```cpp
class MultipleGamepadManager {
private:
    std::vector<std::unique_ptr<GamepadDevice>> m_devices;
    ComPtr<IDirectInput8> m_directInput;
public:
    void ScanForDevices();
    void ProcessAllDevices();
    GamepadDevice* FindDeviceByName(const std::wstring& name);
};
```

### 3. 設定ファイル形式の拡張

#### 現在の形式
```json
{
  "gamepad": { ... },
  "config": { ... }
}
```

#### 新しい形式（デバイス識別付き）
```json
{
  "device_info": {
    "name": "Xbox Controller",
    "instance_name": "Controller (Xbox One For Windows)",
    "guid": "{...}"
  },
  "gamepad": { ... },
  "config": { ... }
}
```

### 4. 実装順序

#### Phase 1: 基盤クラスの作成
1. `GamepadDevice` クラス実装
2. `MultipleGamepadManager` クラス実装
3. デバイス識別機能

#### Phase 2: 設定管理の拡張
1. デバイス固有設定ファイル
2. 設定ファイル命名規則
3. 自動設定生成機能

#### Phase 3: Application層の修正
1. `Application` クラスの複数デバイス対応
2. 入力処理ループの並列化
3. ログ出力の改善

#### Phase 4: テストと最適化
1. 複数デバイス同時接続テスト
2. 設定ファイル管理テスト
3. パフォーマンス最適化

### 5. 互換性の考慮

#### 既存設定ファイルの移行
- `gamepad_config.json` が存在する場合の自動移行
- デフォルトデバイス設定として使用

#### ログの統合
- 複数デバイスの情報を1つのログに統合
- デバイス識別情報をログに含める

### 6. 期待される動作

#### デバイス検出
```
[INFO] Game controller detected: Xbox Controller (Device 1)
[INFO] Game controller detected: PS4 Controller (Device 2)
[INFO] Loading config: gamepad_config_Xbox_Controller.json
[INFO] Loading config: gamepad_config_PS4_Controller.json
```

#### 同時入力処理
```
[Xbox] Button0 -> VKseq PRESSED
[PS4] Button1 -> VKseq PRESSED
```

#### 設定ファイル自動生成
- 新しいデバイスが接続された場合
- デバイス名に基づく設定ファイルの自動作成
- デフォルト設定の適用

## まとめ

現在の実装は完全に単一デバイス前提で作られているため、複数ゲームパッド対応には大幅なアーキテクチャ変更が必要です。主要な変更点は：

1. **デバイス管理の多重化**: 単一 → 複数デバイス管理
2. **設定ファイルの個別化**: グローバル → デバイス固有設定
3. **入力処理の並列化**: 単一処理 → 複数デバイス同時処理

これらの変更により、複数のゲームパッドを同時に使用し、それぞれ個別の設定で動作させることが可能になります。