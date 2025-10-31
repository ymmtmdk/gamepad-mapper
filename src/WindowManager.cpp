#include "WindowManager.h"
#include "Logger.h"
#include "DisplayBuffer.h"
#include "resource.h"

WindowManager::WindowManager(HINSTANCE hInstance, const std::wstring& title)
    : m_hInst(hInstance), m_title(title), m_logger(nullptr) {}

WindowManager::WindowManager(HINSTANCE hInstance, const std::wstring& title, ILogger* logger)
    : m_hInst(hInstance), m_title(title), m_logger(logger) {}

WindowManager::WindowManager(HINSTANCE hInstance, const std::wstring& title, IDisplayBuffer* displayBuffer)
    : m_hInst(hInstance), m_title(title), m_displayBuffer(displayBuffer) {}

WindowManager::~WindowManager() {
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
    }
}

bool WindowManager::Init(int width, int height) {
    const wchar_t* clsName = L"DInputMinimalWnd";
    WNDCLASS wc{};
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = m_hInst;
    wc.lpszClassName = clsName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_GAMEPADMAPPER));
    if (!RegisterClass(&wc)) return false;

    m_hWnd = CreateWindowEx(
        0, 
        clsName, 
        m_title.c_str(),
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, 
        nullptr, nullptr, m_hInst, 
        this // Pass `this` pointer to WM_NCCREATE
    );
    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
    return true;
}

LRESULT CALLBACK WindowManager::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WindowManager* pThis = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (WindowManager*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->m_hWnd = hWnd; // Set the handle in the instance
    } else {
        pThis = (WindowManager*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }

    if (pThis) {
        return pThis->MemberWndProc(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WindowManager::MemberWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY: {
        m_running = false;
        PostQuitMessage(0);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);
        HBRUSH hbr = (HBRUSH)(COLOR_WINDOW + 1);
        FillRect(hdc, &rc, hbr);

        // Get display lines from DisplayBuffer
        const auto& logLines = m_displayBuffer ? 
            m_displayBuffer->GetLines() : 
            std::vector<std::wstring>();
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        int y = 4;
        for (const auto& line : logLines) {
            TextOutW(hdc, 4, y, line.c_str(), (int)line.size());
            y += tm.tmHeight + 2;
            if (y > rc.bottom - 10) break;
        }

        EndPaint(hWnd, &ps);
        return 0;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}
