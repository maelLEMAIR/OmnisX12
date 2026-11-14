#ifndef SCENE_CPP_DEFINED
#define SCENE_CPP_DEFINED

#include "Scene.h"
#include "Engine.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::Init(String const& _name, uint32 _id)
{
    m_name = _name;
    m_id = _id;

    OnInit();
}

void Scene::Update(float _dt)
{
    OnUpdate(_dt);
    
    Renderer* renderer = EngineManager::GetInstance().GetRenderer();
    
    if (renderer != nullptr)
        renderer->BeginDraw();
    
    world.Update(_dt);
    
    if (renderer != nullptr)
        renderer->EndDraw();
}

#endif
