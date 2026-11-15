#ifndef CAMERA_CPP_INCLUDED
#define CAMERA_CPP_INCLUDED

#include "Camera.h"

void Camera::UpdateViewMatrix()
{
    XMFLOAT4X4& worldMatrix = transform.GetMatrix();

    m_view = XMMatrixInverse(
        nullptr,
        XMLoadFloat4x4(&worldMatrix)
    );

    UpdateViewProj();
}

Camera::Camera(XMFLOAT3 const& pos)
{
    transform = Transform();
    transform.pos = pos;
    transform.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

void Camera::SetOrthographic(float32 left, float32 right, float32 bottom, float32 top, float32 nearZ, float32 farZ)
{
    m_type = ORTHOGRAPHIC;
    m_proj = XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearZ, farZ);
    UpdateViewProj();
}

void Camera::SetPerspective(float32 fovY, float32 aspect, float32 nearZ, float32 farZ)
{
    m_type = PERSPECTIVE;
    m_proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
    UpdateViewProj();
}

#endif