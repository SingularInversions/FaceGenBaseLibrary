//#pragma once

cbuffer ConstantFrameBuffer : register(b0)
{
	struct
	{

        float4x4 WorldViewMatrix;
        float4x4 ProjectMatrix;
        float4   AmbientColor;
        float4   LightDirection[2]; 
        float4   LightColor[2];

	} FrameBuffer;
}


uint PackColor(float4 color) {
    return (uint(color.r * 255) << 24) | (uint(color.g * 255) << 16) | (uint(color.b * 255) << 8) | uint(color.a * 255);
}

float4 UnpackColor(uint color) {
    float4 result;
    result.r = float((color >> 24) & 0x000000ff) / 255.0f;
    result.g = float((color >> 16) & 0x000000ff) / 255.0f;
    result.b = float((color >> 8) & 0x000000ff) / 255.0f;
    result.a = float(color & 0x000000ff) / 255.0f;
    return saturate(result);
}