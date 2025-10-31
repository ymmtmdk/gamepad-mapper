# GamepadMapper ドキュメント構成

このフォルダには、GamepadMapperプロジェクトの各種ドキュメントが含まれています。

## 📚 プロダクトドキュメント（開発者・ユーザー向け）

### メインドキュメント（推奨）
- **[API_REFERENCE.md](API_REFERENCE.md)** - APIクイックリファレンス・使用例
- **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** - 開発環境構築・実践的開発ガイド
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - よくある問題と解決法
- **[CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md)** - 設定ファイル詳細ガイド

### 詳細ドキュメント（必要時のみ）
*注意: 詳細版ドキュメント（ORIGINAL版）は簡略化作業中にバックアップとして保存されました。現在のメインドキュメントで情報が不足する場合は、内部資料から復元可能です。*

---

## 🔍 プロジェクト内部資料（管理・分析用）

### 分析・調査レポート
- **[DOCUMENT_IMPLEMENTATION_ANALYSIS.md](internal/DOCUMENT_IMPLEMENTATION_ANALYSIS.md)** - ドキュメント実装整合性分析
- **[DOCUMENTATION_SIMPLIFICATION_REPORT.md](internal/DOCUMENTATION_SIMPLIFICATION_REPORT.md)** - ドキュメント簡略化作業レポート
- **[LOGGER_MIGRATION_PLAN.md](internal/LOGGER_MIGRATION_PLAN.md)** - Logger移行計画・実行レポート
- **[MIGRATION_SUCCESS_SUMMARY.md](internal/MIGRATION_SUCCESS_SUMMARY.md)** - 移行成功サマリー
- **[CONFIGURATION_FILES_CREATION_REPORT.md](internal/CONFIGURATION_FILES_CREATION_REPORT.md)** - 設定ファイル作成レポート

### 技術分析・戦略文書
- **[LOG_SYSTEM_SEPARATION_ANALYSIS.md](internal/LOG_SYSTEM_SEPARATION_ANALYSIS.md)** - ログシステム分離分析
- **[LOGGER_IMPROVEMENT_ANALYSIS.md](internal/LOGGER_IMPROVEMENT_ANALYSIS.md)** - ログ改善分析
- **[REFACTORING_STRATEGY.md](internal/REFACTORING_STRATEGY.md)** - リファクタリング戦略

---

## 🎯 ドキュメント使い分けガイド

### 👥 プロダクトドキュメント利用者
- **新規開発者**: メインドキュメントから開始
- **既存開発者**: APIリファレンス・設定ガイドを活用
- **ユーザー**: 設定ガイド・トラブルシューティングを参照
- **詳細調査時**: ORIGINAL版を参照

### 🔧 内部資料利用者
- **プロジェクトマネージャー**: 分析レポートで改善状況確認
- **テクニカルリード**: 技術戦略文書で方針決定
- **品質保証**: 移行・改善レポートで品質確認
- **将来の改善作業**: 過去の分析を参考に計画立案

---

## 📁 ファイル移動計画

以下のファイルを `docs/internal/` フォルダに移動予定：

### 移動対象（内部資料）
```
docs/
├── DOCUMENT_IMPLEMENTATION_ANALYSIS.md → internal/
├── DOCUMENTATION_SIMPLIFICATION_REPORT.md → internal/
├── LOGGER_MIGRATION_PLAN.md → internal/
├── MIGRATION_SUCCESS_SUMMARY.md → internal/
├── CONFIGURATION_FILES_CREATION_REPORT.md → internal/
├── LOG_SYSTEM_SEPARATION_ANALYSIS.md → internal/
├── LOGGER_IMPROVEMENT_ANALYSIS.md → internal/
└── REFACTORING_STRATEGY.md → internal/
```

### 保持対象（プロダクトドキュメント）
```
docs/
├── API_REFERENCE.md ✓
├── DEVELOPER_GUIDE.md ✓
├── TROUBLESHOOTING.md ✓
├── CONFIGURATION_GUIDE.md ✓
├── API_REFERENCE_ORIGINAL.md ✓
├── DEVELOPER_GUIDE_ORIGINAL.md ✓
└── TROUBLESHOOTING_ORIGINAL.md ✓
```

---

**次のアクション**: ドキュメント移動とフォルダ構成の整理を実行