#include "EnhancedRenderer.h"
#include "MultipleGamepadManager.h"
#include "GamepadDevice.h"
#include "ILogger.h"
#include <algorithm>
#include <numeric>
#include <sstream>

EnhancedRenderer::EnhancedRenderer()
    : m_boldFont(nullptr)
    , m_regularFont(nullptr)
    , m_borderPen(nullptr)
    , m_panelBrush(nullptr)
    , m_lastUpdateTime(0)
{
    CreateGDIResources();
}

void EnhancedRenderer::CreateGDIResources()
{
    // Create fonts
    m_regularFont = CreateFont(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    
    m_boldFont = CreateFont(
        14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );

    // Create drawing resources
    m_borderPen = CreatePen(PS_SOLID, 1, UIColors::BORDER_GRAY);
    m_panelBrush = CreateSolidBrush(UIColors::PANEL_LIGHT);
}

void EnhancedRenderer::CleanupGDIResources()
{
    if (m_regularFont) DeleteObject(m_regularFont);
    if (m_boldFont) DeleteObject(m_boldFont);
    if (m_borderPen) DeleteObject(m_borderPen);
    if (m_panelBrush) DeleteObject(m_panelBrush);
}

void EnhancedRenderer::Render(HDC hdc, const RECT& clientRect,
                             const MultipleGamepadManager* gamepadManager,
                             const ILogger* logger)
{
    // Calculate layout
    CalculateLayout(clientRect);

    // Set background mode for text
    SetBkMode(hdc, TRANSPARENT);

    // Process data
    auto devices = ProcessDeviceStatus(gamepadManager);
    auto events = ProcessInputEvents(logger);
    
    size_t connectedCount = std::count_if(devices.begin(), devices.end(),
        [](const DeviceStatusDisplay& d) { return d.isConnected; });
    
    size_t activeInputCount = std::accumulate(devices.begin(), devices.end(), 0u,
        [](size_t sum, const DeviceStatusDisplay& d) { return sum + d.activeInputs; });

    // Render all panels
    RenderHeader(hdc, m_headerRect, connectedCount, devices.size(), activeInputCount);
    RenderDevicePanel(hdc, m_devicePanelRect, devices);
    RenderInputMonitor(hdc, m_inputMonitorRect, events);
    
    if (logger) {
        RenderEventLog(hdc, m_eventLogRect, logger->GetFrameLog());
    }
}

void EnhancedRenderer::CalculateLayout(const RECT& clientRect)
{
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Header at top
    m_headerRect = {
        UILayout::MARGIN,
        UILayout::MARGIN,
        width - UILayout::MARGIN,
        UILayout::MARGIN + UILayout::HEADER_HEIGHT
    };

    // Device panel on left
    m_devicePanelRect = {
        UILayout::MARGIN,
        m_headerRect.bottom + UILayout::MARGIN,
        UILayout::MARGIN + UILayout::DEVICE_PANEL_WIDTH,
        height - UILayout::LOG_PANEL_HEIGHT - UILayout::MARGIN * 2
    };

    // Input monitor on right
    m_inputMonitorRect = {
        m_devicePanelRect.right + UILayout::MARGIN,
        m_headerRect.bottom + UILayout::MARGIN,
        width - UILayout::MARGIN,
        height - UILayout::LOG_PANEL_HEIGHT - UILayout::MARGIN * 2
    };

    // Event log at bottom
    m_eventLogRect = {
        UILayout::MARGIN,
        height - UILayout::LOG_PANEL_HEIGHT - UILayout::MARGIN,
        width - UILayout::MARGIN,
        height - UILayout::MARGIN
    };
}

void EnhancedRenderer::RenderHeader(HDC hdc, const RECT& rect,
                                   size_t connectedDevices, size_t totalDevices, size_t activeInputs)
{
    DrawPanel(hdc, rect, UIColors::HEADER_DARK, UIColors::BORDER_GRAY);

    // Title and status
    HFONT oldFont = (HFONT)SelectObject(hdc, m_boldFont);
    
    std::wstring title = L"🎮 GamepadMapper v2.0 - Enhanced UI";
    DrawColoredText(hdc, rect.left + UILayout::PADDING, rect.top + UILayout::PADDING, 
                   title, UIColors::TEXT_WHITE, true);

    std::wstring status = L"📊 Connected: " + std::to_wstring(connectedDevices) + 
                         L"/" + std::to_wstring(totalDevices) + 
                         L" │ Active Inputs: " + std::to_wstring(activeInputs);
    
    DrawColoredText(hdc, rect.right - 300, rect.top + UILayout::PADDING, 
                   status, UIColors::TEXT_WHITE);

    SelectObject(hdc, oldFont);
}

void EnhancedRenderer::RenderDevicePanel(HDC hdc, const RECT& rect,
                                        const std::vector<DeviceStatusDisplay>& devices)
{
    DrawPanel(hdc, rect, UIColors::PANEL_LIGHT, UIColors::BORDER_GRAY);

    HFONT oldFont = (HFONT)SelectObject(hdc, m_boldFont);
    DrawColoredText(hdc, rect.left + UILayout::PADDING, rect.top + UILayout::PADDING,
                   L"🎮 Connected Devices", UIColors::TEXT_BLACK, true);
    SelectObject(hdc, m_regularFont);

    int y = rect.top + UILayout::HEADER_HEIGHT;
    for (size_t i = 0; i < devices.size() && y < rect.bottom - 20; ++i) {
        const auto& device = devices[i];
        
        // Draw device icon
        DrawDeviceIcon(hdc, rect.left + UILayout::PADDING, y, 
                      device.isConnected, device.deviceColor);

        // Device name
        COLORREF textColor = device.isConnected ? UIColors::TEXT_BLACK : UIColors::INACTIVE_GRAY;
        std::wstring deviceText = device.name;
        if (device.isConnected) {
            deviceText += L" ✅";
        } else {
            deviceText += L" ❌";
        }
        
        DrawColoredText(hdc, rect.left + UILayout::PADDING + 20, y, 
                       deviceText, textColor);

        // Active inputs count
        if (device.isConnected && device.activeInputs > 0) {
            std::wstring inputsText = L"Active: " + std::to_wstring(device.activeInputs);
            DrawColoredText(hdc, rect.left + UILayout::PADDING + 20, y + 16,
                           inputsText, UIColors::ACTIVE_GREEN);
        }

        y += 35;
    }

    SelectObject(hdc, oldFont);
}

void EnhancedRenderer::RenderInputMonitor(HDC hdc, const RECT& rect,
                                         const std::vector<InputEventDisplay>& events)
{
    DrawPanel(hdc, rect, UIColors::PANEL_LIGHT, UIColors::BORDER_GRAY);

    HFONT oldFont = (HFONT)SelectObject(hdc, m_boldFont);
    DrawColoredText(hdc, rect.left + UILayout::PADDING, rect.top + UILayout::PADDING,
                   L"🔄 Live Input Monitor", UIColors::TEXT_BLACK, true);
    SelectObject(hdc, m_regularFont);

    int y = rect.top + UILayout::HEADER_HEIGHT;
    for (const auto& event : events) {
        if (y >= rect.bottom - 20) break;

        // Draw input mapping
        DrawInputMapping(hdc, rect.left + UILayout::PADDING, y,
                        event.inputType + L" " + event.inputDetail,
                        event.mappedKey, event.isActive);

        // Device name with color
        std::wstring deviceInfo = L"[" + event.deviceName + L"]";
        DrawColoredText(hdc, rect.left + UILayout::PADDING + 250, y,
                       deviceInfo, event.deviceColor);

        y += 20;
    }

    SelectObject(hdc, oldFont);
}

void EnhancedRenderer::RenderEventLog(HDC hdc, const RECT& rect,
                                     const std::vector<std::wstring>& logLines)
{
    DrawPanel(hdc, rect, UIColors::PANEL_LIGHT, UIColors::BORDER_GRAY);

    HFONT oldFont = (HFONT)SelectObject(hdc, m_boldFont);
    DrawColoredText(hdc, rect.left + UILayout::PADDING, rect.top + UILayout::PADDING,
                   L"📝 Event Log", UIColors::TEXT_BLACK, true);
    SelectObject(hdc, m_regularFont);

    int y = rect.top + UILayout::HEADER_HEIGHT;
    size_t startLine = logLines.size() > 6 ? logLines.size() - 6 : 0;
    
    for (size_t i = startLine; i < logLines.size() && y < rect.bottom - 20; ++i) {
        const auto& line = logLines[i];
        
        // Color-code based on content
        COLORREF textColor = UIColors::TEXT_BLACK;
        if (line.find(L"ACTIVE") != std::wstring::npos) {
            textColor = UIColors::ACTIVE_GREEN;
        } else if (line.find(L"Connected") != std::wstring::npos) {
            textColor = UIColors::CONNECTED_BLUE;
        } else if (line.find(L"Disconnected") != std::wstring::npos) {
            textColor = UIColors::DISCONNECTED_RED;
        }

        DrawColoredText(hdc, rect.left + UILayout::PADDING, y, line, textColor);
        y += 18;
    }

    SelectObject(hdc, oldFont);
}

// Helper methods implementation
void EnhancedRenderer::DrawPanel(HDC hdc, const RECT& rect, COLORREF bgColor, COLORREF borderColor)
{
    HBRUSH bgBrush = CreateSolidBrush(bgColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, borderColor);
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bgBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bgBrush);
    DeleteObject(borderPen);
}

void EnhancedRenderer::DrawStatusIndicator(HDC hdc, int x, int y, bool active, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(active ? color : UIColors::INACTIVE_GRAY);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    
    Ellipse(hdc, x, y, x + 10, y + 10);
    
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void EnhancedRenderer::DrawDeviceIcon(HDC hdc, int x, int y, bool connected, COLORREF deviceColor)
{
    DrawStatusIndicator(hdc, x, y, connected, deviceColor);
}

void EnhancedRenderer::DrawInputMapping(HDC hdc, int x, int y, const std::wstring& input,
                                       const std::wstring& output, bool active)
{
    COLORREF color = active ? UIColors::ACTIVE_GREEN : UIColors::INACTIVE_GRAY;
    
    std::wstring mapping = input + L" → " + output;
    if (active) {
        mapping += L" [ACTIVE]";
    }
    
    DrawColoredText(hdc, x, y, mapping, color);
}

void EnhancedRenderer::DrawColoredText(HDC hdc, int x, int y, const std::wstring& text,
                                      COLORREF color, bool bold)
{
    COLORREF oldColor = SetTextColor(hdc, color);
    HFONT oldFont = nullptr;
    
    if (bold && m_boldFont) {
        oldFont = (HFONT)SelectObject(hdc, m_boldFont);
    }
    
    TextOutW(hdc, x, y, text.c_str(), (int)text.length());
    
    if (oldFont) {
        SelectObject(hdc, oldFont);
    }
    SetTextColor(hdc, oldColor);
}

std::vector<DeviceStatusDisplay> EnhancedRenderer::ProcessDeviceStatus(const MultipleGamepadManager* gamepadManager)
{
    std::vector<DeviceStatusDisplay> devices;
    
    if (!gamepadManager) {
        return devices;
    }

    auto allNames = gamepadManager->GetAllDeviceNames();
    auto connectedNames = gamepadManager->GetConnectedDeviceNames();
    
    for (size_t i = 0; i < allNames.size(); ++i) {
        DeviceStatusDisplay device;
        device.name = allNames[i];
        device.instanceName = L""; // Could be enhanced to get instance name
        device.isConnected = std::find(connectedNames.begin(), connectedNames.end(), 
                                     allNames[i]) != connectedNames.end();
        device.activeInputs = 0; // TODO: Implement active input counting
        device.deviceColor = GetDeviceColor(i);
        
        devices.push_back(device);
    }
    
    return devices;
}

std::vector<InputEventDisplay> EnhancedRenderer::ProcessInputEvents(const ILogger* logger)
{
    std::vector<InputEventDisplay> events;
    
    if (!logger) {
        return events;
    }

    // Parse log lines to extract input events
    const auto& logLines = logger->GetFrameLog();
    for (const auto& line : logLines) {
        // Parse different types of input events
        if (line.find(L"Button") != std::wstring::npos && line.find(L"PRESSED") != std::wstring::npos) {
            InputEventDisplay event;
            event.inputType = L"Button";
            event.isActive = true;
            event.deviceColor = UIColors::DEVICE_1_BLUE; // Default
            event.timestamp = GetTickCount();
            
            // TODO: Parse device name, button number, and mapped key from log line
            events.push_back(event);
        }
    }
    
    return events;
}

COLORREF EnhancedRenderer::GetDeviceColor(size_t deviceIndex)
{
    switch (deviceIndex % 4) {
        case 0: return UIColors::DEVICE_1_BLUE;
        case 1: return UIColors::DEVICE_2_GREEN;
        case 2: return UIColors::DEVICE_3_RED;
        case 3: return UIColors::DEVICE_4_PURPLE;
        default: return UIColors::DEVICE_1_BLUE;
    }
}