#ifndef APPLICATION_CPP_INCLUDED
#define APPLICATION_CPP_INCLUDED

#include "Application.h"
#include <stdexcept>
#include <string>

Application::Application(HINSTANCE instance)
    : m_window(L"OmnisX12", 1280, 720)
{
    if (!m_window.Create())
        throw std::runtime_error("Window creation failed");

    m_renderer.Init(m_window.Handle(), m_window.Width(), m_window.Height());

    m_window.OnResize = [this](int w, int h) { m_renderer.OnResize(w, h); };
}

int Application::Run()
{
    while (m_window.PumpMessages())
    {
        m_chrono.Tick();

        if (m_window.IsMinimized())
        {
            Sleep(50);
            continue;
        }

        const float dt = m_chrono.DeltaTime();
        m_renderer.Update(m_chrono);
        
        m_renderer.BeginDraw();
        // m_renderer.Draw();
        m_renderer.EndDraw();
        UpdateFpsInTitle(dt);
    }
    return 0;
}

void Application::UpdateFpsInTitle(float dt)
{
    m_fpsTime += dt;
    ++m_fpsFrames;
    if (m_fpsTime >= 1.0f)
    {
        m_window.SetTitle(L"OmnisX12 - " + std::to_wstring(m_fpsFrames) + L" FPS");
        m_fpsTime   = 0.0f;
        m_fpsFrames = 0;
    }
}

#endif
