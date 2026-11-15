#ifndef RESOURCE_MANAGER_H_DEFINED
#define RESOURCE_MANAGER_H_DEFINED

#include "define.h"
#include "../Render/D3D12/Items/Geometry.h"
#include "../Render/D3D12/Camera.h"

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	// TODO RAW DATA STORAGE
	
	static uint32 AddGeometry(String _name, Geometry* _pGeo);
	static uint32 GetGeometryId(String _name) { return s_pInstance->m_mGeometryIds[_name]; };
	static Geometry* GetGeometry(String _name);
	static Geometry* GetGeometry(uint32 _id) { return s_pInstance->m_vGeometries[_id]; }

	static uint32 AddShader(String const& _name, uint32& _pShader);
	static uint32 GetShaderId(String _name) { return s_pInstance->m_mShaderIds[_name]; }

	static uint32 AddCamera(String _name);
	static uint32 GetCameraId(String _name) { return s_pInstance->m_mCameraIds[_name]; };
	static Camera* GetCamera(String _name);
	static Camera* GetCamera(uint32 _id) { return s_pInstance->m_vCameras[_id]; }
	
private:
	static ResourceManager* s_pInstance;

	Vector<Geometry*> m_vGeometries;
	Vector<Camera*> m_vCameras;
	
	UnorderedMap<String, uint32> m_mGeometryIds;
	UnorderedMap<String, uint32> m_mShaderIds;
	UnorderedMap<String, uint32> m_mCameraIds;
};

#endif