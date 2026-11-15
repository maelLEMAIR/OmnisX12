#include "CameraSystem.h"

#include "ResourceManager.h"
#include "Render/D3D12/Renderer.h"
#include "../EngineManager.h"
#include "ECS/World.h"

void CameraSystem::Update(World& world, float _dt)
{
    world.Query<CameraComponent, TransformComponent>([&](CameraComponent& _camera, TransformComponent& _transform)
    {
        Camera* cam = ResourceManager::GetCamera(_camera.camId);
        if (cam == nullptr) return;
        
        Renderer* renderer = EngineManager::GetInstance().GetRenderer();
        
        cam->transform = _transform.world;
        if (_camera.isMainCamera)
            renderer->SetMainCamera(cam);
        
    });
}
