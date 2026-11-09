#ifndef CHRONO_CPP_DEFINED
#define CHRONO_CPP_DEFINED

#include "Chrono.h"

Chrono::Chrono() : m_lastTick(Clock::now()) {}

void Chrono::Tick()
{
    const Clock::time_point now = Clock::now();
    const double elapsed = std::chrono::duration<double>(now - m_lastTick).count();
    m_lastTick = now;

    if (m_paused)
    {
        m_deltaTime = 0.0f;
        return;
    }

    m_deltaTime = static_cast<float>(elapsed < m_maxDelta ? elapsed : m_maxDelta);
    m_totalTime += m_deltaTime;
}

void Chrono::Pause()  { m_paused = true; }

void Chrono::Resume()
{
    if (!m_paused) return;
    m_paused = false;
    m_lastTick = Clock::now();  
}

void Chrono::Reset()
{
    m_totalTime = 0.0;
    m_deltaTime = 0.0f;
    m_lastTick  = Clock::now();
}

#endif
