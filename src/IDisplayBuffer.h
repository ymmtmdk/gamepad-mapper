#pragma once
#include <vector>
#include <string>
#include <dinput.h>

/**
 * @brief スクリーン表示専用のバッファインターフェース
 * 
 * ファイル出力とは完全に分離された、スクリーン表示のみを担当するインターフェース。
 * WindowManagerがこのインターフェースを通じて表示データを取得する。
 */
class IDisplayBuffer {
public:
    virtual ~IDisplayBuffer() = default;

    // バッファ管理
    virtual void Clear() = 0;
    virtual void SetMaxLines(size_t maxLines) = 0;
    virtual size_t GetMaxLines() const = 0;

    // 基本的なテキスト追加
    virtual void AddLine(const std::wstring& line) = 0;
    virtual void AddFormattedLine(const wchar_t* fmt, ...) = 0;

    // ゲームパッド専用の表示メソッド
    virtual void AddGamepadHeader(const std::wstring& deviceName) = 0;
    virtual void AddGamepadInfo(bool connected, 
                               const std::wstring& productName, 
                               const std::wstring& instanceName) = 0;
    virtual void AddGamepadState(const std::wstring& deviceName, 
                                const DIJOYSTATE2& state) = 0;

    // ステータス情報表示
    virtual void AddStatusLine(const std::wstring& status) = 0;
    virtual void AddSeparator() = 0;

    // データアクセス
    virtual const std::vector<std::wstring>& GetLines() const = 0;
    virtual size_t GetLineCount() const = 0;
    virtual bool IsEmpty() const = 0;

    // 統計情報
    virtual size_t GetTotalLinesAdded() const = 0;
    virtual void ResetStatistics() = 0;

    // 設定オプション
    virtual void SetTimestampEnabled(bool enabled) = 0;
    virtual bool IsTimestampEnabled() const = 0;
    virtual void SetAutoSeparator(bool enabled) = 0;
    virtual bool IsAutoSeparatorEnabled() const = 0;
};

/**
 * @brief ゲームパッドの状態情報を格納する構造体
 */
struct GamepadDisplayInfo {
    std::wstring deviceName;
    std::wstring productName;
    std::wstring instanceName;
    bool isConnected = false;
    DIJOYSTATE2 currentState = {};
};

/**
 * @brief 表示用のステータス情報
 */
struct DisplayStatus {
    size_t connectedDevices = 0;
    size_t totalDevices = 0;
    std::vector<std::wstring> connectedNames;
    std::vector<std::wstring> disconnectedNames;
    std::wstring currentMode;
};