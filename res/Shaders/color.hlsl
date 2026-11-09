//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
    float value = 0.0f;
};

struct VertexIn
{
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
    // Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
    // Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float3 HSVtoRGB(float h, float s, float v)
{
    float c = v * s;
    float hPrime = h / 60.0;
    float x = c * (1.0 - abs(fmod(hPrime, 2.0) - 1.0));
    float m = v - c;

    float3 rgb;

    if (h < 60.0)
        rgb = float3(c, x, 0);
    else if (h < 120.0)
        rgb = float3(x, c, 0);
    else if (h < 180.0)
        rgb = float3(0, c, x);
    else if (h < 240.0)
        rgb = float3(0, x, c);
    else if (h < 300.0)
        rgb = float3(x, 0, c);
    else
        rgb = float3(c, 0, x);

    return rgb + m;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}
