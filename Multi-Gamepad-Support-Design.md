# 複数ゲームパッド対応 設計ドキュメント

本ドキュメントは、アプリケーションに複数のゲームパッドを同時に接続し、それぞれに個別の設定を適用するための設計について記述する。

## 1. 要求事項

- **複数デバイスのサポート**: 複数のゲームパッドを同時に認識し、それぞれを独立して動作させる。
- **個別設定**: ゲームパッドごとに異なるキーマッピング設定を個別のファイルで管理できるようにする。
- **デバイス識別**: ゲームパッドを製品名などの一意な情報で識別し、対応する設定ファイルを自動で読み込む。
- **ログの一元化**: ログ出力は従来通り、単一のシステムに集約する。

## 2. 設計方針

中心的なアイデアは、**ゲームパッドの製品名**と**設定ファイル名**を関連付け、デバイスごとに独立した入力処理のパイプラインを構築することである。

### 2.1. ゲームパッドの識別

- `DirectInputManager` は、デバイス列挙時に各ゲームパッドの **製品名 (Product Name)** を取得する。
- この製品名は、デバイスを一意に識別するためのキーとして使用する。
- 製品名に含まれる可能性のあるファイルシステムで不正な文字は、アンダースコア `_` などに置換（サニタイズ）する。

### 2.2. 設定ファイルの管理

- 設定ファイルは、新設する `configs/` ディレクトリ内に格納する。
- 設定ファイル名は、**サニタイズ後のデバイス製品名**に `.json` の拡張子を付けたものとする。
  - 例: `Controller (XBOX 360 For Windows)` → `configs/Controller_XBOX_360_For_Windows.json`
- `JsonConfigManager` を拡張、または新しい `ConfigLoader` クラスを導入し、起動時に `configs/` ディレクトリ内のすべての `.json` ファイルを読み込む。
- ロードされた設定は、ファイル名をキーとする `std::map` で管理する。

### 2.3. アーキテクチャの変更

主要モジュールの責務を以下のように変更・拡張する。

#### `DirectInputManager`

- **責務**: デバイスの列挙、管理、および各デバイスに対応する `InputProcessor` の統括。
- **変更点**:
    - 内部に `std::map<std::string, LPDIRECTINPUTDEVICE8>` を保持し、製品名と `DirectInput` デバイスをマッピングする。
    - `std::vector<std::unique_ptr<InputProcessor>>` を保持し、アクティブなゲームパッドの入力プロセッサを管理する。
    - 初期化時に、検出した各デバイスに対応する設定ファイルを `ConfigManager` から取得し、`InputProcessor` を生成・初期化する。
    - メインループから呼び出される `updateAllDevices()` メソッドを新設し、すべての `InputProcessor` の処理を実行する。

#### `JsonConfigManager` (または `ConfigLoader`)

- **責務**: 複数の設定ファイルの読み込みと管理。
- **変更点**:
    - `load(path)` を `loadConfigs(directoryPath)` に変更し、ディレクトリ内の全設定を読み込む。
    - `getConfigForDevice(deviceName)` のようなインターフェースを提供し、デバイス名に基づいて適切な設定情報を返す。

#### `InputProcessor`

- **責務**: **単一の**ゲームパッドデバイスの入力状態を監視し、キーボードイベントに変換する。
- **変更点**:
    - コンストラクタで、処理対象の `LPDIRECTINPUTDEVICE8` デバイスインスタンスと、適用するキーマッピング設定（`GamepadConfig` など）を受け取る。
    - 状態管理やキー送信のロジックは、担当するデバイスと設定にのみ関心を持つ。

### 2.4. 処理フロー

1.  **アプリケーション起動**:
    - `ConfigManager` が `configs/` ディレクトリからすべての設定ファイルを読み込み、キャッシュする。
2.  **初期化**:
    - `DirectInputManager` がゲームパッドを列挙し、製品名を取得する。
    - 列挙した各デバイスについて、`DirectInputManager` は以下の処理を行う。
        a. 製品名をサニタイズする。
        b. サニタイズした製品名を使って `ConfigManager` に設定を問い合わせる。
        c. 設定が存在すれば、その設定とデバイスインスタンスを使って `InputProcessor` を生成する。
        d. 生成した `InputProcessor` を管理下のリストに追加する。
3.  **メインループ**:
    - `WinMain.cpp` のメインループは `DirectInputManager::updateAllDevices()` を呼び出す。
    - `DirectInputManager` は、管理しているすべての `InputProcessor` の `processInput()` メソッドを順に呼び出す。
    - 各 `InputProcessor` は、担当デバイスの入力状態をポーリングし、変更があれば設定に基づいてキーボードイベントを送信する。

## 3. ファイル構成の変更

```
WindowsProject1/
├── configs/                      # (新設) 個別設定ファイル用ディレクトリ
│   ├── X-Box_360_Controller.json
│   └── Some_Other_Gamepad.json
├── WinMain.cpp
├── JsonConfigManager.{h,cpp}     # (変更) 複数ファイル対応
├── DirectInputManager.{h,cpp}    # (変更) 複数デバイス管理
├── InputProcessor.{h,cpp}        # (変更) 単一デバイス責務
...
```
