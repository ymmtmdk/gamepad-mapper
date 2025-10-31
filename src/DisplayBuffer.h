#pragma once
#include <windows.h>
#include <dinput.h>
#include <mutex>
#include <chrono>

/**
 * @brief スクリーン表示専用のバッファ実装
 * 
 * スレッドセーフなスクリーン表示バッファ。メモリ制限機能付きで、
 * 古い行を自動的に削除して無制限な蓄積を防ぐ。
 */
class DisplayBuffer {
public:
    // コンストラクタ/デストラクタ
    DisplayBuffer(size_t maxLines = 100);

    // DisplayBuffer interface implementation
    void Clear();
    void SetMaxLines(size_t maxLines);
    size_t GetMaxLines() const;

    void AddLine(const std::wstring& line);
    void AddFormattedLine(const wchar_t* fmt, ...);

    void AddGamepadHeader(const std::wstring& deviceName);
    void AddGamepadInfo(bool connected, 
                       const std::wstring& productName, 
                       const std::wstring& instanceName);
    void AddGamepadState(const std::wstring& deviceName, 
                        const DIJOYSTATE2& state);

    void AddStatusLine(const std::wstring& status);
    void AddSeparator();

    const std::vector<std::wstring>& GetLines() const;
    size_t GetLineCount() const;
    bool IsEmpty() const;

    size_t GetTotalLinesAdded() const;
    void ResetStatistics();

    void SetAutoSeparator(bool enabled);
    bool IsAutoSeparatorEnabled() const;

private:
    // Copy operations disabled
    DisplayBuffer(const DisplayBuffer&) = delete;
    DisplayBuffer& operator=(const DisplayBuffer&) = delete;

    // Internal helper methods
    void AddLineInternal(const std::wstring& line);
    void TrimToMaxLines();
    std::wstring FormatGamepadState(const DIJOYSTATE2& state);
    std::wstring FormatButtonState(const DIJOYSTATE2& state);
    std::wstring GetTimestamp();

    // Member variables
    std::vector<std::wstring> m_lines;
    size_t m_maxLines;
    size_t m_totalLinesAdded;
    bool m_timestampEnabled;
    bool m_autoSeparatorEnabled;
    mutable std::mutex m_mutex;

    // Constants for formatting
    static constexpr size_t DEFAULT_MAX_LINES = 100;
    static constexpr size_t MIN_MAX_LINES = 10;
    static constexpr size_t MAX_MAX_LINES = 1000;
};