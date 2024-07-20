#include "Common.hlsl"

// Pipeline resources
Texture2D<float4> tex : register(t0, Space_PerMaterial);
SamplerState clampPoint : register(s0, Space_Constant);

cbuffer Camera : register(b0, Space_PerObject)
{
    row_major float4x4 ModelMatrix : packoffset(c0);
    row_major float4x4 ViewMatrix : packoffset(c4);
    row_major float4x4 ProjectionMatrix : packoffset(c8);
};

float4 TransformPosition(in float3 aPosition)
{
    const float4 worldPosition = mul(float4(aPosition, 1.0f), ModelMatrix);
    const float4 cameraSpacePosition = mul(worldPosition, ViewMatrix);
    const float4 projectionPosition = mul(cameraSpacePosition, ProjectionMatrix);
    
    return projectionPosition;
}

struct FretboardVertex
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct PixelData
{
    float4 ScreenPosition : SV_Position;
    float2 UV : TEXCOORD;
};

PixelData vertexShader(FretboardVertex anInput)
{
    PixelData pixelData;

    pixelData.ScreenPosition = TransformPosition(anInput.Position);
    pixelData.UV = anInput.UV;

    return pixelData;
}

float4 pixelShader(PixelData anInput) : SV_TARGET
{
    const float3 textureColor = tex.Sample(clampPoint, anInput.UV).rgb;
    return float4(textureColor, 1.0f);
}
