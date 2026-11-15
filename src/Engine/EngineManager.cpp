#ifndef ENGINE_MANAGER_CPP_DEFINED
#define ENGINE_MANAGER_CPP_DEFINED

#include "EngineManager.h"
#include "Engine.h"
#include "Scene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "Render/D3D12/Resources/GeometryFactory.hpp"
#include "Systems/TransformSystem.h"



EngineManager* EngineManager::s_pInstance = nullptr;

EngineManager::EngineManager()
{
    s_pInstance = this;
    m_chrono = Chrono();
}

EngineManager::~EngineManager()
{
    Exit();
    if (s_pInstance == this)
        s_pInstance = nullptr;
}

EngineManager& EngineManager::GetInstance()
{
    if (s_pInstance == nullptr)
        s_pInstance = new EngineManager();

    return *s_pInstance;
}

void EngineManager::Exit()
{
}

void EngineManager::Initialize(UINT _width, UINT _height, std::wstring _title, bool _fullscreen, int argc, char* argv[])
{
    if (m_pSceneManager == nullptr)
        m_pSceneManager = new SceneManager;
    
    if (m_pWindow == nullptr)
    {
        m_pWindow = new Window(_title, (int)_width, (int)_height);
        if ( !m_pWindow->Create() )
            throw std::runtime_error("Window creation failed");
        
        if ( _fullscreen)
            m_pWindow->ToggleFullScreen();
    }
    
    if (m_pRenderer == nullptr)
    {
        m_pRenderer = new Renderer();
        m_pRenderer->Init(m_pWindow->Handle(), (int)_width, (int)_height);
        m_pDevice = &m_pRenderer->GetGpuDevice();
    }

    m_pRenderer->SetClearColor(ToColor(87, 185, 255));
    m_pResourceManager = new ResourceManager;

    ResourceManager::AddGeometry("Cube", GeometryFactory::BuildCube());
    ResourceManager::AddGeometry("Sphere", GeometryFactory::BuildUVSphere(18, 18));
    ResourceManager::AddGeometry("Grid", GeometryFactory::BuildGrid(20, 20));
    
    uint32 flatShader = m_pRenderer->CreateShader("Flat", RES("Shaders/flat.hlsl"));
    ResourceManager::AddShader("Flat", flatShader);
    uint32 toonShader = m_pRenderer->CreateShader("Toon", RES("Shaders/toon.hlsl"));
    ResourceManager::AddShader("Toon", toonShader);
    
    uint32 camId = ResourceManager::AddCamera("Default");
    Camera* cam  = ResourceManager::GetCamera(camId);
    cam->SetPerspective(0.25f * XM_PI, (float)_width / (float)_height, 1.0f, 1000.0f);
    InputManager::Initialize(m_pWindow->Handle());
}

void EngineManager::Run()
{
    while (m_pWindow->PumpMessages() && m_pWindow->IsOpen())
    {
        m_chrono.Tick();

        if (m_pWindow->IsMinimized())
        {
            Sleep(50);
            continue;
        }

        m_deltaTime = m_chrono.DeltaTime();
        m_pRenderer->Update(m_chrono);
        InputManager::Update(m_deltaTime);
        m_pSceneManager->GetCurrentScene()->Update(m_deltaTime);
        UpdateFpsInTitle(m_deltaTime);
    }
}

void EngineManager::UpdateFpsInTitle(float dt)
{
    m_fpsTime += dt;
    ++m_fpsFrames;
    if (m_fpsTime >= 1.0f)
    {
        m_pWindow->SetTitle(L"OmnisX12 - " + std::to_wstring(m_fpsFrames) + L" FPS");
        m_fpsTime   = 0.0f;
        m_fpsFrames = 0;
    }
}

#endif
