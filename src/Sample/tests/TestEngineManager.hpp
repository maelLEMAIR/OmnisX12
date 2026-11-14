#ifndef TEST_ENGINE_MANAGER_HPP_DEFINED
#define TEST_ENGINE_MANAGER_HPP_DEFINED

#include "InputManager.h"
#include "Test.h"
#include "Engine/Engine.h"
#include "Render/D3D12/Resources/GeometryFactory.hpp"

class MainScene : public Scene
{
    TransformComponent* transformComp = nullptr;
    
public:
    void OnInit() override
    {
        EntityId e1 = world.CreateEntity();
        MeshRenderer& renderer = world.AddComponent<MeshRenderer>(e1);
        renderer.geoId = ResourceManager::GetGeometryId("Cube");
        renderer.color   = { 1.0f, 0.2f, 0.2f, 1.0f };
        // renderer.materialId = RessourceManager::GetMaterialId("Default");
        transformComp = world.GetComponent<TransformComponent>(e1);
        transformComp->local.SetPosition({ 0.0f, 0.0f, 5.0f });

        EntityId camera = world.CreateEntity();
        TransformComponent& t = world.AddComponent<TransformComponent>(camera);
        t.local.SetPosition(XMFLOAT3(0.0f, 5.0f, -5.0f));
        t.local.AddYPR({0.0f, XM_PI / 8, 0.0f});
        t.SetParent(e1);
        CameraComponent& cam = world.AddComponent<CameraComponent>(camera);
        cam.camId = ResourceManager::GetCameraId("Default");
        cam.isMainCamera = true;
        
        EntityId e2 = world.CreateEntity();
        MeshRenderer& rendererE2 = world.AddComponent<MeshRenderer>(e2);
        rendererE2.geoId = ResourceManager::GetGeometryId("Cube");
        rendererE2.color = { 0.4f, 0.4f, 0.4f, 1.0f };
        // rendererE2.materialId = RessourceManager::GetMaterialId("Red");
        TransformComponent* transformE2 = world.GetComponent<TransformComponent>(e2);
        transformE2->local.SetPosition({ 0.0f, -5.0f, 0.0f });
        transformE2->local.SetScale({ 10.0f, 1.0f, 10.0f });
        
        Scene::OnInit();
    }

    void OnUpdate(float _dt) override
    {
        if (InputManager::IsKeyPressed(ENTER))
            EngineManager::GetRenderer()->SetClearColor({255.0f, 255.0f, 255.0f});
        if (InputManager::IsKeyPressed(DELETE_))
            EngineManager::GetRenderer()->SetClearColor({255.0f, 0.0f, 0.0f});
        if (InputManager::IsKeyPressed(ESCAPE))
            EngineManager::GetWindow()->Close();
    }
    
    void OnEnd() override { }
    void OnStart() override { }
    void LoadResources() override { }
};

class TestEngineManager : public Test
{
public: 
    static void Run()
    {
        EngineManager engineManager;
        engineManager.Initialize(1920, 1080, L"TestEngineManager", true);

        SceneManager::CreateSceneType<MainScene>("MainScene");
        SceneManager::SetCurrentScene("MainScene");

        engineManager.Run();
    }
};


#endif