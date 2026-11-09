#pragma once
#include <chrono>
#include <cstdint>

class Chrono
{
public:
    Chrono();
    
    void Tick();

    void Pause();
    void Resume();
    void Reset();     

    float  DeltaTime()  const { return m_deltaTime; }          
    double TotalTime()  const { return m_totalTime; }     
    bool   IsPaused()   const { return m_paused; }

    void  SetMaxDelta(float seconds) { m_maxDelta = seconds; }

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point m_lastTick;
    double m_totalTime = 0.0;
    float  m_deltaTime = 0.0f;
    float  m_maxDelta  = 0.25f;
    bool   m_paused    = false;
};