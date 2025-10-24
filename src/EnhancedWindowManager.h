#pragma once
#include "WindowManager.h"
#include "EnhancedRenderer.h"
#include <memory>

// Forward declarations
class MultipleGamepadManager;

class EnhancedWindowManager : public WindowManager {
public:
    EnhancedWindowManager(HINSTANCE hInstance, const std::wstring& title);
    EnhancedWindowManager(HINSTANCE hInstance, const std::wstring& title, ILogger* logger);
    ~EnhancedWindowManager();

    // Set gamepad manager reference for enhanced rendering
    void SetGamepadManager(MultipleGamepadManager* gamepadManager);

    // Override window procedure for enhanced rendering
    LRESULT MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    std::unique_ptr<EnhancedRenderer> m_renderer;
    MultipleGamepadManager* m_gamepadManager;
    
    // Enhanced paint handling
    void HandleEnhancedPaint(HWND hWnd);
};