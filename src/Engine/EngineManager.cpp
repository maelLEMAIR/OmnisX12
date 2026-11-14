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
    
    // Shader* coloredS = ShaderFactory::CreateLitColored(m_pDevice);
    // RessourceManager::AddShader("Color", coloredS);
    //
    // Material* white = coloredS->CreateMaterial();
    // white->SetFloat4("DiffuseAlbedo", {1.0f, 1.0f, 1.0f, 1.0f});
    // RessourceManager::AddMaterial("Default", white);
    //
    // Material* red = coloredS->CreateMaterial();
    // red->SetFloat4("DiffuseAlbedo", {1.0f, 0.0f, 0.0f, 1.0f});
    // RessourceManager::AddMaterial("Red", red);
    //
    // RessourceManager::AddCamera("Default");
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
    }
}

#endif
