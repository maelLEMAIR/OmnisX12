#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED

#include "../Resources/d3d12Utils.h"

struct Light
{
    XMFLOAT3 Strength;
    XMFLOAT3 direction;
    XMFLOAT3 position;
    float FalloffStart;
    float FalloffEnd;
    float SpotPower;
};

#endif