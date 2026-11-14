#ifndef MESH_RENDERER_SYSTEM_CPP_INCLUDED
#define MESH_RENDERER_SYSTEM_CPP_INCLUDED

#include "MeshRendererSystem.h"

#include "EngineManager.h"
#include "ResourceManager.h"
#include "../Components/MeshRenderer.hpp"
#include "../Components/TransformComponent.hpp"
#include "ECS/World.h"

void MeshRendererSystem::Update(World& world, float deltaTime)
{
    Renderer* renderer = EngineManager::GetRenderer();

    world.Query<MeshRenderer, TransformComponent>([&](MeshRenderer& mesh, TransformComponent& transform)
    {
        Geometry* geo = ResourceManager::GetGeometry(mesh.geoId);
        if (geo == nullptr) return;

        MeshGeometry* gpuGeo = renderer->GetOrCreateGeometry(mesh.geoId, *geo);
        if (gpuGeo == nullptr) return;

        renderer->Draw(*gpuGeo, transform.world.GetMatrix(), mesh.color);
    });
}

#endif