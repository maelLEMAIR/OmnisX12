#ifndef TEST_ENGINE_MANAGER_HPP_DEFINED
#define TEST_ENGINE_MANAGER_HPP_DEFINED

#include "InputManager.h"
#include "Test.h"
#include "Engine/Engine.h"
#include "Render/D3D12/Resources/GeometryFactory.hpp"

class MainScene : public Scene
{
    EntityId m_camera;
    
public:
    void OnInit() override
    {
        EntityId e1 = world.CreateEntity();
        MeshRenderer& renderer = world.AddComponent<MeshRenderer>(e1);
        renderer.geoId = ResourceManager::GetGeometryId("Sphere");
        renderer.color   = { 1.0f, 0.2f, 0.2f, 1.0f };
        renderer.shaderId  = ResourceManager::GetShaderId("Toon"); 
        // renderer.materialId = RessourceManager::GetMaterialId("Default");
        TransformComponent* transformComp = world.GetComponent<TransformComponent>(e1);
        transformComp->local.SetPosition({ 0.0f, 0.0f, 0.0f });

        m_camera = world.CreateEntity();
        TransformComponent* _t = world.GetComponent<TransformComponent>(m_camera);
        _t->local.SetPosition(XMFLOAT3(0.0f, 5.0f, -5.0f));
        _t->local.LookAt({ 0.0f, 0.0f, 5.0f });
        CameraComponent& cam = world.AddComponent<CameraComponent>(m_camera);
        cam.camId = ResourceManager::GetCameraId("Default");
        cam.isMainCamera = true;
        
        EntityId e2 = world.CreateEntity();
        MeshRenderer& rendererE2 = world.AddComponent<MeshRenderer>(e2);
        rendererE2.geoId = ResourceManager::GetGeometryId("Cube");
        rendererE2.color = { 0.4f, 0.4f, 0.4f, 1.0f };
        rendererE2.shaderId = ResourceManager::GetShaderId("Toon"); 
        // rendererE2.materialId = RessourceManager::GetMaterialId("Red");
        TransformComponent* transformE2 = world.GetComponent<TransformComponent>(e2);
        transformE2->local.SetPosition({ 0.0f, -5.0f, 0.0f });
        transformE2->local.SetScale({ 10.0f, 1.0f, 10.0f });
        
        Scene::OnInit();
    }

    void OnUpdate(float _dt) override
    {
        TransformComponent* _t = world.GetComponent<TransformComponent>(m_camera);
        if (_t == nullptr) return;
        
        float speed = 5.0f;
        float speedRot = 10.0f;
        if ( InputManager::IsKeyPressed(Z) )
            _t->local.Move(_t->local.forward, speed * _dt);
        if ( InputManager::IsKeyPressed(S) )
            _t->local.Move(_t->local.forward, -speed * _dt);
        if ( InputManager::IsKeyPressed(Q) )
            _t->local.Move(_t->local.right, -speed * _dt);
        if ( InputManager::IsKeyPressed(D) )
            _t->local.Move(_t->local.right, speed * _dt);
        if ( InputManager::IsKeyPressed(LCTRL) )
            _t->local.Move(_t->local.up, -speed * _dt);
        if ( InputManager::IsKeyPressed(SPACEBAR) )
            _t->local.Move(_t->local.up, speed * _dt);
        
        if ( InputManager::IsKeyPressed(LEFT_ARROW) )
            _t->local.AddYPR({XM_PI * -speedRot / 32.0f * _dt, 0.0f, 0.0f});
        if ( InputManager::IsKeyPressed(RIGHT_ARROW) )
            _t->local.AddYPR({XM_PI * speedRot / 32.0f * _dt, 0.0f, 0.0f});
        if ( InputManager::IsKeyPressed(UP_ARROW) )
            _t->local.AddYPR({0.0f, XM_PI * -speedRot / 32.0f * _dt, 0.0f});
        if ( InputManager::IsKeyPressed(DOWN_ARROW) )
            _t->local.AddYPR({0.0f, XM_PI * speedRot / 32.0f * _dt, 0.0f});
        
        
        if (InputManager::IsKeyPressed(ENTER))
            EngineManager::GetRenderer()->SetClearColor({255.0f, 255.0f, 255.0f});
        if (InputManager::IsKeyPressed(DELETE_))
            EngineManager::GetRenderer()->SetClearColor({255.0f, 0.0f, 0.0f});
        if (InputManager::IsKeyPressed(ESCAPE))
            EngineManager::GetWindow()->Close();
        
        if ( InputManager::IsKeyPressed(F11) )
            EngineManager::GetWindow()->ToggleFullScreen();
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