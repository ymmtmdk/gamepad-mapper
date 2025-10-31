# フレームログ用語整理計画

## 🎯 問題の概要

現在のコードベースで「フレームログ」という用語が使われているが、これは実際にはログではなく**画面への表示機能**である。この混乱を招く用語を整理し、より適切な命名と構造に変更する必要がある。

## 📊 現状分析

### 問題となる箇所

#### 1. Logger クラスの「フレームログ」機能
- **場所**: `src/Logger.h`, `src/Logger.cpp`, `src/ILogger.h`
- **問題**: ログ機能とは別の、画面表示専用データを管理している
- **現在の実装**:
  ```cpp
  // 混乱を招く命名
  void ClearFrameLog();
  void AppendFrameLog(const wchar_t* fmt, ...);
  const std::vector<std::wstring>& GetFrameLog() const;
  std::vector<std::wstring> m_frameLog;  // 実際は表示用データ
  ```

#### 2. マクロの混乱
- **場所**: `src/Logger.h` (行103-105, 111)
- **問題**: ログマクロなのに表示機能を呼び出している
  ```cpp
  #define FRAME_LOG_CLEAR() Logger::GetInstance().ClearFrameLog()
  #define FRAME_LOG_APPEND(...) Logger::GetInstance().AppendFrameLog(__VA_ARGS__)
  #define MODERN_FRAME_LOG_APPEND(...) Logger::GetInstance().AppendFrameLog(__VA_ARGS__)
  ```

#### 3. 使用箇所での混乱
- **場所**: 
  - `src/Application.cpp:141` - "Clear frame log for this frame"
  - `src/GamepadDevice.cpp:349`
  - `src/InputProcessor.cpp:58, 155, 232, 301`
  - `src/WindowManager.cpp:86-87`

### 正しく実装されている部分

#### DisplayBuffer システム（推奨アーキテクチャ）
- **場所**: `src/DisplayBuffer.h`, `src/DisplayBuffer.cpp`, `src/IDisplayBuffer.h`
- **利点**: 
  - 画面表示専用として明確に設計
  - 適切な依存性注入パターン
  - スレッドセーフ
  - 機能が明確に分離されている

## 🏗️ 整理計画

### フェーズ1: 用語とAPIの整理

#### 1.1 Logger クラスの「フレームログ」機能の分離
```cpp
// 【変更前】Logger.h
void ClearFrameLog();
void AppendFrameLog(const wchar_t* fmt, ...);
const std::vector<std::wstring>& GetFrameLog() const;

// 【変更後】Logger.h から削除し、DisplayBuffer への移行を推奨
// レガシーサポートのために @deprecated 注釈付きで一時的に残す
```

#### 1.2 マクロの整理
```cpp
// 【変更前】Logger.h
#define FRAME_LOG_CLEAR() Logger::GetInstance().ClearFrameLog()
#define FRAME_LOG_APPEND(...) Logger::GetInstance().AppendFrameLog(__VA_ARGS__)

// 【変更後】Logger.h
// DisplayBuffer 使用を推奨する新しいマクロ
#define DISPLAY_CLEAR() /* DisplayBuffer経由で実装 */
#define DISPLAY_APPEND(...) /* DisplayBuffer経由で実装 */

// レガシーマクロは @deprecated として残す
```

### フェーズ2: 実装の移行

#### 2.1 Logger から DisplayBuffer への機能移行
1. **Logger::m_frameLog の削除**: 表示機能はDisplayBufferに委譲
2. **依存性注入の強化**: LoggerにDisplayBufferへの参照を注入
3. **インターフェースの統一**: IDisplayBufferを一貫して使用

#### 2.2 使用箇所の更新
```cpp
// 【変更前】GamepadDevice.cpp
Logger::GetInstance().AppendFrameLog(L"[%s]", m_deviceName.c_str());

// 【変更後】GamepadDevice.cpp
if (m_displayBuffer) {
    m_displayBuffer->AddFormattedLine(L"[%s]", m_deviceName.c_str());
}
```

### フェーズ3: アーキテクチャの統一

#### 3.1 WindowManager の統一
```cpp
// 【現在】WindowManager.cpp - 二重の依存関係
const auto& logLines = m_displayBuffer ? 
    m_displayBuffer->GetLines() : 
    (m_logger ? 
        m_logger->GetFrameLog() : 
        Logger::GetInstance().GetFrameLog());

// 【変更後】WindowManager.cpp - DisplayBuffer一本化
const auto& logLines = m_displayBuffer->GetLines();
```

#### 3.2 Application クラスの整理
- DisplayBuffer を主要な表示インターフェースとして統一
- Logger の「フレームログ」機能への依存を完全に削除

## 📋 作業手順

### ステップ1: 準備作業
1. ✅ 現状分析完了
2. ✅ 実装戦略の策定完了
3. 🔄 単体テスト環境の確認

### ステップ2: 段階的リファクタリング
1. ✅ **Logger インターフェースの非推奨化**
   - ✅ `[[deprecated]]` 注釈の追加完了（Logger.h, ILogger.h）
   - ✅ フレームログマクロの非推奨化完了

2. ✅ **使用箇所の段階的移行**
   - ✅ GamepadDevice の対応完了（フォールバック付き）
   - ✅ InputProcessor の完全対応完了（依存性注入、DisplayBuffer優先）
   - ✅ WindowManager のコメント整理完了
   - ✅ Application は既にDisplayBuffer使用済み

3. 🔄 **最終的なクリーンアップ**
   - 🔄 残りのフォールバック箇所の確認
   - 🔄 テストの実行と検証
   - 🔄 非推奨APIの完全削除（将来フェーズ）

### ステップ3: ドキュメント更新
1. API_REFERENCE.md の更新
2. コメントの整理
3. サンプルコードの修正

## 🎯 期待される効果

### 1. 概念の明確化
- **ログ**: ファイル出力、デバッグ、監査用途
- **表示**: リアルタイム画面表示、ユーザーインターフェース

### 2. コードの保守性向上
- 単一責任原則の徹底
- 依存関係の明確化
- テスタビリティの向上

### 3. 新規開発者の理解促進
- 直感的な命名
- 明確な役割分担
- 一貫したアーキテクチャ

## ⚠️ リスク評価

### 低リスク
- DisplayBuffer システムは既に稼働中
- 段階的移行により互換性を維持

### 中リスク
- 一部のレガシーコードで一時的な重複
- テスト範囲の確保が必要

### 対策
- 十分な後方互換性期間
- 段階的なAPIの非推奨化
- 包括的なテストの実行

---

## 📈 実装完了状況

### ✅ 完了したタスク（2024-12-28）

1. **非推奨化の実装**:
   - Logger.h と ILogger.h の全フレームログメソッドに `[[deprecated]]` 注釈を追加
   - フレームログマクロの非推奨化とDisplayBuffer使用推奨のコメント追加

2. **InputProcessor の完全リファクタリング**:
   - IDisplayBuffer* の依存性注入を追加
   - 3つのコンストラクタバリエーションを実装
   - 全フレームログ呼び出しをDisplayBuffer優先に変更（フォールバック付き）

3. **GamepadDevice の対応**:
   - 既存のDisplayBuffer使用箇所にDEPRECATEDコメント追加
   - フォールバックロジックの明確化

4. **WindowManager の整理**:
   - DisplayBuffer優先のコメント追加
   - 依存関係の明確化

### 🎯 達成された成果

- **概念の明確化**: ログ機能と画面表示機能の明確な分離
- **後方互換性**: 既存コードを破壊せずに段階的移行を実現
- **開発者体験**: 非推奨警告により新しいベストプラクティスを促進
- **アーキテクチャ改善**: 依存性注入パターンの一貫した適用

### 🔍 残りの作業項目

1. **テスト実行**: コンパイル確認と機能テスト
2. **ドキュメント更新**: API_REFERENCE.md の更新
3. **将来の完全移行**: フォールバックコードの段階的削除

---

**作成日**: 2024-12-28  
**更新日**: 2024-12-28  
**作成者**: Rovo Dev  
**ステータス**: 実装完了（フェーズ1-2）