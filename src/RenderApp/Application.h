#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include "Window.h"
#include "D3D12/Renderer.h"
#include "Core/Chrono.h"

class Application
{
public:
    explicit Application(HINSTANCE instance);
    int Run();

private:
    void UpdateFpsInTitle(float dt);

    Window   m_window;
    Renderer m_renderer;
    Chrono   m_chrono;

    float m_fpsTime   = 0.0f;
    int   m_fpsFrames = 0;
};

#endif
