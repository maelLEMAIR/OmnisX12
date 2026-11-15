#ifndef RESOURCE_MANAGER_CPP_DEFINED
#define RESOURCE_MANAGER_CPP_DEFINED

#include "ResourceManager.h"

ResourceManager* ResourceManager::s_pInstance = nullptr;

ResourceManager::ResourceManager()
{
    s_pInstance = this;
}

ResourceManager::~ResourceManager()
{
}

uint32 ResourceManager::AddGeometry(String _name, Geometry* _pGeo)
{
    assert(s_pInstance->m_mGeometryIds.contains(_name) == false && "Geometry already exists");
    
    s_pInstance->m_vGeometries.push_back(_pGeo);
    s_pInstance->m_mGeometryIds[_name] = s_pInstance->m_vGeometries.size() - 1;
    return s_pInstance->m_mGeometryIds[_name];
}

Geometry* ResourceManager::GetGeometry(String _name)
{
    if (s_pInstance->m_mGeometryIds.contains(_name) == false) return nullptr;

    return s_pInstance->m_vGeometries[s_pInstance->m_mGeometryIds[_name]];
}

uint32 ResourceManager::AddShader(String const& _name, uint32& _pShader)
{
    assert(s_pInstance->m_mShaderIds.contains(_name) == false && "Shader already exists");
    
    s_pInstance->m_mShaderIds[_name] = _pShader;
    return _pShader;
}

uint32 ResourceManager::AddCamera(String _name)
{
    assert(s_pInstance->m_mCameraIds.contains(_name) == false && "Camera already exists");
    
    s_pInstance->m_vCameras.push_back(new Camera());
    s_pInstance->m_mCameraIds[_name] = s_pInstance->m_vCameras.size() - 1;
    return s_pInstance->m_mCameraIds[_name];
}

Camera* ResourceManager::GetCamera(String _name)
{
    if (s_pInstance->m_mCameraIds.contains(_name) == false) return nullptr;

    return s_pInstance->m_vCameras[s_pInstance->m_mCameraIds[_name]];
}

#endif