#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <windows.h>
#include <functional>
#include <string>

class Window
{
public:
    Window(HINSTANCE instance, const std::wstring& title, int clientWidth, int clientHeight);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Create();

    bool PumpMessages();

    HWND Handle()      const { return m_hwnd; }
    int  Width()       const { return m_width; }
    int  Height()      const { return m_height; }
    bool IsMinimized() const { return m_minimized; }
    void SetTitle(const std::wstring& title);

    std::function<void(int, int)> OnResize;

private:
    static LRESULT CALLBACK WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void NotifyResize();

    HINSTANCE    m_instance;
    HWND         m_hwnd = nullptr;
    std::wstring m_title;
    int  m_width;
    int  m_height;
    int  m_notifiedWidth  = 0;
    int  m_notifiedHeight = 0;
    bool m_minimized = false;
    bool m_resizing  = false;
};

#endif