#pragma once
#include <windows.h>
#include <string>
#include "Logger.h"
#include "DisplayBuffer.h"

class WindowManager {
public:
    WindowManager(HINSTANCE hInstance, const std::wstring& title);
    WindowManager(HINSTANCE hInstance, const std::wstring& title, Logger* logger);
    WindowManager(HINSTANCE hInstance, const std::wstring& title, DisplayBuffer* displayBuffer);
    ~WindowManager();

    bool Init(int width, int height);
    HWND GetHwnd() const { return m_hWnd; }
    bool IsRunning() const { return m_running; }
    void SetRunning(bool running) { m_running = running; }
    
    // Dependency injection
    void SetLogger(Logger* logger) { m_logger = logger; }
    void SetDisplayBuffer(DisplayBuffer* displayBuffer) { m_displayBuffer = displayBuffer; }

private:
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd = nullptr;
    std::wstring m_title;
    bool m_running = true;
    Logger* m_logger = nullptr; // Injected dependency (legacy)
    DisplayBuffer* m_displayBuffer = nullptr; // Injected dependency (new)
};
