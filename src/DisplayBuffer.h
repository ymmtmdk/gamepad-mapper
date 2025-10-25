#pragma once
#include "IDisplayBuffer.h"
#include <mutex>
#include <chrono>

/**
 * @brief スクリーン表示専用のバッファ実装
 * 
 * スレッドセーフなスクリーン表示バッファ。メモリ制限機能付きで、
 * 古い行を自動的に削除して無制限な蓄積を防ぐ。
 */
class DisplayBuffer : public IDisplayBuffer {
public:
    // コンストラクタ/デストラクタ
    DisplayBuffer(size_t maxLines = 100);
    ~DisplayBuffer() override = default;

    // IDisplayBuffer interface implementation
    void Clear() override;
    void SetMaxLines(size_t maxLines) override;
    size_t GetMaxLines() const override;

    void AddLine(const std::wstring& line) override;
    void AddFormattedLine(const wchar_t* fmt, ...) override;

    void AddGamepadHeader(const std::wstring& deviceName) override;
    void AddGamepadInfo(bool connected, 
                       const std::wstring& productName, 
                       const std::wstring& instanceName) override;
    void AddGamepadState(const std::wstring& deviceName, 
                        const DIJOYSTATE2& state) override;

    void AddStatusLine(const std::wstring& status) override;
    void AddSeparator() override;

    const std::vector<std::wstring>& GetLines() const override;
    size_t GetLineCount() const override;
    bool IsEmpty() const override;

    size_t GetTotalLinesAdded() const override;
    void ResetStatistics() override;

    // 追加機能
    void SetTimestampEnabled(bool enabled);
    bool IsTimestampEnabled() const;

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