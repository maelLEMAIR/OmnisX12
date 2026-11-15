#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "Core/Transform.h"
#include "Core/MathUtils.hpp"

enum CameraType
{
    ORTHOGRAPHIC,
    PERSPECTIVE
};

class Camera
{
public:
    Camera(XMFLOAT3 const& pos = { 0.0f, 0.0f, 0.0f });
    void SetOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 nearZ, float32 farZ);
    void SetPerspective(float32 fovAngleY, float32 aspectRatio, float32 nearZ, float32 farZ);

    XMFLOAT4X4 const& GetMatrix()       const { return transform.matrix; }
    XMMATRIX const& GetViewMatrix()     const { return m_view; }
    XMMATRIX const& GetProjMatrix()     const { return m_proj; }
    XMMATRIX const& GetVPMatrix()       const { return m_viewProj; }
    
    XMFLOAT3 GetPosition()          const { return transform.pos; }
    XMFLOAT4 GetRotation()          const { return transform.quat; }
    
    void UpdateViewMatrix();
    
    CameraType GetType() const { return m_type; }

    Transform transform;
    
private:
    void UpdateViewProj() { m_viewProj = m_view * m_proj; }

    XMMATRIX m_view     = XMMatrixIdentity();
    XMMATRIX m_proj     = XMMatrixIdentity();
    XMMATRIX m_viewProj = XMMatrixIdentity();
    CameraType m_type = PERSPECTIVE;
};

#endif
