#include "EnhancedWindowManager.h"
#include "MultipleGamepadManager.h"
#include "Logger.h"

EnhancedWindowManager::EnhancedWindowManager(HINSTANCE hInstance, const std::wstring& title)
    : WindowManager(hInstance, title)
    , m_renderer(std::make_unique<EnhancedRenderer>())
    , m_gamepadManager(nullptr)
{
}

EnhancedWindowManager::EnhancedWindowManager(HINSTANCE hInstance, const std::wstring& title, ILogger* logger)
    : WindowManager(hInstance, title, logger)
    , m_renderer(std::make_unique<EnhancedRenderer>())
    , m_gamepadManager(nullptr)
{
}

EnhancedWindowManager::~EnhancedWindowManager() = default;

void EnhancedWindowManager::SetGamepadManager(MultipleGamepadManager* gamepadManager)
{
    m_gamepadManager = gamepadManager;
}

LRESULT EnhancedWindowManager::MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_DESTROY:
        m_running = false;
        PostQuitMessage(0);
        return 0;
        
    case WM_PAINT:
        HandleEnhancedPaint(hWnd);
        return 0;
        
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

void EnhancedWindowManager::HandleEnhancedPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    // Clear background
    HBRUSH bgBrush = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(hdc, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    // Use enhanced renderer
    if (m_renderer) {
        const ILogger* logger = m_logger ? m_logger : &Logger::GetInstance();
        m_renderer->Render(hdc, clientRect, m_gamepadManager, logger);
    }

    EndPaint(hWnd, &ps);
}