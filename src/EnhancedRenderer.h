#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class MultipleGamepadManager;
class ILogger;

// UI Colors for enhanced visualization
namespace UIColors {
    constexpr COLORREF ACTIVE_GREEN = RGB(0, 255, 0);
    constexpr COLORREF INACTIVE_GRAY = RGB(128, 128, 128);
    constexpr COLORREF CONNECTED_BLUE = RGB(0, 100, 255);
    constexpr COLORREF DISCONNECTED_RED = RGB(255, 50, 50);
    constexpr COLORREF HEADER_DARK = RGB(40, 40, 40);
    constexpr COLORREF TEXT_WHITE = RGB(255, 255, 255);
    constexpr COLORREF TEXT_BLACK = RGB(0, 0, 0);
    constexpr COLORREF PANEL_LIGHT = RGB(240, 240, 240);
    constexpr COLORREF BORDER_GRAY = RGB(200, 200, 200);
    
    // Device-specific colors
    constexpr COLORREF DEVICE_1_BLUE = RGB(0, 120, 255);
    constexpr COLORREF DEVICE_2_GREEN = RGB(0, 200, 100);
    constexpr COLORREF DEVICE_3_RED = RGB(255, 80, 80);
    constexpr COLORREF DEVICE_4_PURPLE = RGB(180, 80, 255);
}

// UI Layout constants
namespace UILayout {
    constexpr int MARGIN = 8;
    constexpr int PADDING = 4;
    constexpr int HEADER_HEIGHT = 30;
    constexpr int DEVICE_PANEL_WIDTH = 200;
    constexpr int STATUS_PANEL_HEIGHT = 60;
    constexpr int LOG_PANEL_HEIGHT = 150;
}

// Enhanced rendering structure for input events
struct InputEventDisplay {
    std::wstring deviceName;
    std::wstring inputType;     // "Button", "D-Pad", "Stick"
    std::wstring inputDetail;   // "A", "Up", "Left"
    std::wstring mappedKey;     // "Z", "↑", "A"
    bool isActive;
    DWORD timestamp;
    COLORREF deviceColor;
};

// Device status display structure
struct DeviceStatusDisplay {
    std::wstring name;
    std::wstring instanceName;
    bool isConnected;
    size_t activeInputs;
    COLORREF deviceColor;
};

class EnhancedRenderer {
public:
    EnhancedRenderer();
    ~EnhancedRenderer() = default;

    // Main rendering method
    void Render(HDC hdc, const RECT& clientRect, 
                const MultipleGamepadManager* gamepadManager,
                const ILogger* logger);

    // Individual panel rendering methods
    void RenderHeader(HDC hdc, const RECT& rect, 
                     size_t connectedDevices, size_t totalDevices, size_t activeInputs);
    
    void RenderDevicePanel(HDC hdc, const RECT& rect, 
                          const std::vector<DeviceStatusDisplay>& devices);
    
    void RenderInputMonitor(HDC hdc, const RECT& rect, 
                           const std::vector<InputEventDisplay>& events);
    
    void RenderEventLog(HDC hdc, const RECT& rect, 
                       const std::vector<std::wstring>& logLines);

    // Helper drawing methods
    void DrawPanel(HDC hdc, const RECT& rect, COLORREF bgColor, COLORREF borderColor);
    void DrawStatusIndicator(HDC hdc, int x, int y, bool active, COLORREF color);
    void DrawDeviceIcon(HDC hdc, int x, int y, bool connected, COLORREF deviceColor);
    void DrawInputMapping(HDC hdc, int x, int y, const std::wstring& input, 
                         const std::wstring& output, bool active);

    // Text rendering with colors
    void DrawColoredText(HDC hdc, int x, int y, const std::wstring& text, 
                        COLORREF color, bool bold = false);
    void DrawCenteredText(HDC hdc, const RECT& rect, const std::wstring& text, 
                         COLORREF color, bool bold = false);

    // Data processing methods
    std::vector<DeviceStatusDisplay> ProcessDeviceStatus(const MultipleGamepadManager* gamepadManager);
    std::vector<InputEventDisplay> ProcessInputEvents(const ILogger* logger);
    
    // Layout calculation
    void CalculateLayout(const RECT& clientRect);

private:
    // Layout rectangles
    RECT m_headerRect;
    RECT m_devicePanelRect;
    RECT m_inputMonitorRect;
    RECT m_eventLogRect;

    // GDI resources
    HFONT m_boldFont;
    HFONT m_regularFont;
    HPEN m_borderPen;
    HBRUSH m_panelBrush;

    // State tracking
    std::vector<InputEventDisplay> m_recentEvents;
    DWORD m_lastUpdateTime;

    // Helper methods
    void CreateGDIResources();
    void CleanupGDIResources();
    COLORREF GetDeviceColor(size_t deviceIndex);
    void UpdateRecentEvents(const std::vector<InputEventDisplay>& newEvents);
    std::wstring FormatTimeStamp(DWORD timestamp);
};