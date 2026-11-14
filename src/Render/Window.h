#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <windows.h>
#include <functional>
#include <string>

#include "Define.h"

class Window
{
public:
    Window(const std::wstring& title, int clientWidth, int clientHeight);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Create();
    bool InitD3D12();
    void ToggleFullScreen();
    void Close() { if (m_hwnd) PostMessage(m_hwnd, WM_CLOSE, 0, 0); }
    bool PumpMessages();

    HWND Handle()      const { return m_hwnd; }
    int  Width()       const { return m_width; }
    int  Height()      const { return m_height; }
    bool IsMinimized() const { return m_minimized; }
    bool IsOpen()      const { return m_isOpen; }
    void SetTitle(const std::wstring& title);

    std::function<void(int, int)> OnResize;

private:
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void NotifyResize();

    HINSTANCE    m_instance;
    HWND         m_hwnd = nullptr;
    
    Renderer* m_pRenderer = nullptr;
    GpuDevice* m_pDevice = nullptr;
    
    tagRECT m_windowedRect;
    
    std::wstring m_title;
    int  m_width;
    int  m_height;
    int  m_notifiedWidth  = 0;
    int  m_notifiedHeight = 0;
    bool m_minimized = false;
    bool m_resizing  = false;
    bool m_isFullScreen = false;
    bool m_isOpen      = false;
};

#endif