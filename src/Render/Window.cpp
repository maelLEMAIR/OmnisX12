#ifndef WINDOW_CPP_INCLUDED
#define WINDOW_CPP_INCLUDED

#include "Window.h"

Window::Window(HINSTANCE instance, const std::wstring& title, int clientWidth, int clientHeight)
    : m_instance(instance), m_title(title), m_width(clientWidth), m_height(clientHeight)
{
}

Window::~Window()
{
    if (m_hwnd)
        DestroyWindow(m_hwnd);
}

bool Window::Create()
{
    WNDCLASS wc = {};
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProcStatic;
    wc.hInstance     = m_instance;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = L"OmnisX12Window";

    if (!RegisterClass(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        MessageBox(nullptr, L"RegisterClass failed.", nullptr, MB_OK);
        return false;
    }

    // On veut une zone CLIENT de la taille demandée : on ajoute bordures et barre de titre.
    RECT rect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    // "this" est passé à CreateWindow et récupéré dans WM_NCCREATE.
    m_hwnd = CreateWindow(L"OmnisX12Window", m_title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, m_instance, this);

    if (!m_hwnd)
    {
        MessageBox(nullptr, L"CreateWindow failed.", nullptr, MB_OK);
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

bool Window::PumpMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Window::SetTitle(const std::wstring& title)
{
    if (m_hwnd)
        SetWindowText(m_hwnd, title.c_str());
}

LRESULT CALLBACK Window::WndProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Window* self = nullptr;
    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = static_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    }
    else
    {
        self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->HandleMessage(hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        m_width     = LOWORD(lParam);
        m_height    = HIWORD(lParam);
        m_minimized = (wParam == SIZE_MINIMIZED);
        
        if (!m_minimized && !m_resizing)
            NotifyResize();
        return 0;

    case WM_ENTERSIZEMOVE:
        m_resizing = true;
        return 0;

    case WM_EXITSIZEMOVE:
        m_resizing = false;
        NotifyResize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_NCDESTROY:
        m_hwnd = nullptr;
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::NotifyResize()
{
    if (m_width == 0 || m_height == 0) return;
    if (m_width == m_notifiedWidth && m_height == m_notifiedHeight) return;

    m_notifiedWidth  = m_width;
    m_notifiedHeight = m_height;
    if (OnResize)
        OnResize(m_width, m_height);
}

#endif
