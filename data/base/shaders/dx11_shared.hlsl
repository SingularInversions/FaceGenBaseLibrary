
// HLSL 5.0

cbuffer ConstantFrameBuffer : register(b0)
{
    struct
    {
        // All members below are in homogeneous representation:
        matrix<float,4,4>   modelview;
        matrix<float,4,4>   projection;
        float4              ambientColor;
        float4              lightDir[2]; 
        float4              lightColor[2];
        // only the first value is used:
        float4              detTexMod;
    } Scene;
}

// t0,t1,t2 correspond to the ordering in PSSetShaderResources:
Texture2D<float4>   albedoMap : register(t0);
Texture2D<float4>   specularMap : register(t1);
Texture2D<float4>   modulationMap : register(t2);

// Binds to the first sampler state as defined by PSSetSamplers:
SamplerState        sstate : register(s0);

struct  Vert
{
    float3  pos :  POSITION;    // Semantic specified in 'IASetInputLayout' to bind this to vertex pos buffer
    float3  norm : NORMAL;      // Semantic binds to vertex normals buffer
    float2  uv :   TEXCOORD;
};

// These values are interpolated from the VS return values of the vertices of the triangle
// being rendered:
struct Frag
{
    // SV_POSITION is an HLSL predefined semantic. The value output by the vertex shader is in
    // clip coords [-1,1] in homogeneous form (pre-division). The pipeline then does the division
    // and converts to screen space [0,dims] before the value is received by the pixel shader:
    float4  pos :  SV_POSITION;
    float4  norm : NORMAL;      // Fragment normal in CCS. 4th component is zero. NORMAL is a predefined semantic
    float2  uv :   TEXCOORD;
};

Frag
VSTransform(Vert v)
{
    Frag    ret;
    ret.pos = mul(Scene.modelview,float4(v.pos,1.0f));
    ret.pos = mul(Scene.projection,ret.pos);
    ret.norm = normalize(mul(Scene.modelview,float4(v.norm,0.0f)));
    ret.uv = v.uv;
    return ret;
}

float3
calcColor(Frag frag,float3 albedo)
{
    float3      specular = specularMap.Sample(sstate,frag.uv).rgb;
    float3      normal = normalize(frag.norm.xyz);              // Points away from viewer for back-facing tris
    float3      color = Scene.ambientColor.rgb * albedo;
    for (uint ii=0; ii<2; ++ii) {
        float3      ld = Scene.lightDir[ii].xyz;
        float3      lc = Scene.lightColor[ii].rgb;
        float       adp = saturate(dot(normal,ld));             // saturate clamps to [0,1]
        color += albedo * lc * adp;                 // Add diffuse term
        float3      halfAngle = normalize(float3(ld[0],ld[1],ld[2]-1));
        float3      del = halfAngle - normal;
        float       d2 = dot(del,del);
        color = saturate(color + specular * lc * exp(-d2*128)); // Add specular term
    }
    return color;
}
