#ifndef ENGINE_MANAGER_H_DEFINED
#define ENGINE_MANAGER_H_DEFINED

// Engine
#include "define.h"

// Core
#include "Core/Chrono.h"

#include "Render/D3D12/Device/GpuDevice.h"

class Renderer;
class Window;
class Scene;
class Camera;
class ResourceManager;
class SceneManager;
class Client;
class Server;

class EngineManager
{
public:
    EngineManager();
    ~EngineManager();

    static EngineManager& GetInstance();
    
    void Initialize(UINT _width, UINT _height, std::wstring _title, bool _fullscreen, int argc = 0, char* argv[] = nullptr);
    void Run();
    void UpdateFpsInTitle(float dt);
    void Exit();
	
    static float GetDeltaTime()     { return GetInstance().m_deltaTime; }
    static Window* GetWindow()      { return GetInstance().m_pWindow; }
    static Renderer* GetRenderer()      { return GetInstance().m_pRenderer; }
    static GpuDevice* GetDevice()   { return GetInstance().m_pDevice; }

private:
    static EngineManager* s_pInstance;

    Chrono m_chrono;
    Chrono m_networkChrono;
    
    float m_deltaTime = 0.0f;
    float m_fpsTime = 0.0f;
    int m_fpsFrames = 0;
    
    Camera* m_camera = nullptr;
    
    Window*     m_pWindow = nullptr;
    Renderer*   m_pRenderer = nullptr;
    GpuDevice*  m_pDevice = nullptr;
    
    SceneManager*       m_pSceneManager = nullptr;
    ResourceManager*   m_pResourceManager = nullptr;
};

#endif

