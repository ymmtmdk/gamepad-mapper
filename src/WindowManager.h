#pragma once
#include <windows.h>
#include <string>
#include "ILogger.h"

class WindowManager {
public:
    WindowManager(HINSTANCE hInstance, const std::wstring& title);
    WindowManager(HINSTANCE hInstance, const std::wstring& title, ILogger* logger);
    ~WindowManager();

    bool Init(int width, int height);
    HWND GetHwnd() const { return m_hWnd; }
    bool IsRunning() const { return m_running; }
    void SetRunning(bool running) { m_running = running; }
    
    // Dependency injection
    void SetLogger(ILogger* logger) { m_logger = logger; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd = nullptr;
    std::wstring m_title;
    bool m_running = true;
    ILogger* m_logger = nullptr; // Injected dependency
};
