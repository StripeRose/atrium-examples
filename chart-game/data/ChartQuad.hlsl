#include "Common.hlsl"

// Pipeline resources
Texture2D<float4> tex : register(t0, Space_PerMaterial);
SamplerState clampLinear : register(s1, Space_Constant);

cbuffer Camera : register(b0, Space_PerFrame)
{
    row_major float4x4 ViewMatrix : packoffset(c0);
    row_major float4x4 ProjectionMatrix : packoffset(c4);
};

struct QuadVertex
{
    float3 BasePosition : POSITION0;

    row_major float4x4 WorldMatrix : TRANSFORM;

    float2 UVMin : TEXCOORD0;
    float2 UVMax : TEXCOORD1;
    float4 Color : COLOR;
};

struct PixelData
{
    float4 ScreenPosition : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

PixelData vertexShader(QuadVertex anInput)
{
    PixelData pixelData;

    const float4 worldPosition = mul(float4(anInput.BasePosition, 1.0f), anInput.WorldMatrix);
    const float4 cameraSpacePosition = mul(worldPosition, ViewMatrix);
    const float4 projectionPosition = mul(cameraSpacePosition, ProjectionMatrix);

    pixelData.ScreenPosition = projectionPosition;
    pixelData.UV = lerp(anInput.UVMin, anInput.UVMax, anInput.BasePosition.xy);
    pixelData.Color = anInput.Color;

    return pixelData;
}

float4 pixelShader(PixelData anInput) : SV_TARGET
{
    const float4 textureColor = tex.Sample(clampLinear, anInput.UV);
    return textureColor * anInput.Color;
}
