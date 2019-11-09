
// t0 and t1 correspond to the ordering in PSSetShaderResources:
Texture2D       albedoMap : register(t0);
Texture2D       specularMap : register(t1);
// For pure dx11 the following must be defined in the code and this binds to the first such definition:
SamplerState    sstate : register(s0);

// Each member size must be a multiple of 8 bytes:
cbuffer Scene
{
    matrix  mvm;
    matrix  projection;
    float4  ambientCol;
    float4  lightDir[2];        // Normalized direction to resp. light in CCS.
    float4  lightColor[2];
}

struct  Vert
{
    float3  pos :  POSITION;     // POSITION specified in 'IASetInputLayout' to bind this to vertex pos buffer
    float3  norm : NORMAL;       // NORMAL " binds to vertex normals buffer
    float2  uv :   TEXCOORD;
};

// These values are interpolated from the VS return values of the vertices of the triangle
// being rendered:
struct Frag
{
    float4  pos :  SV_POSITION; // SV_POSITION is an HLSL predefined semantic
    float4  norm : NORMAL;      // Fragment normal in CCS. 4th component is zero. NORMAL is a predefined semantic
    float2  uv :   TEXCOORD;
};

Frag
VS(Vert v)
{
    Frag    ret;
    ret.pos = mul(mvm,float4(v.pos,1.0f));
    ret.pos = mul(projection,ret.pos);
    ret.norm = normalize(mul(mvm,float4(v.norm,0.0f)));
    ret.uv = v.uv;
    return ret;
}

float4
PS(Frag frag) : SV_Target
{
    float4      alb = albedoMap.Sample(sstate,frag.uv);
    float4      ret = ambientCol * alb;
    float4      norm = normalize(frag.norm);        // Points away from viewer for back-facing tris
    for (uint ii=0; ii<2; ++ii) {
        float4      ld = lightDir[ii];
        float4      lc = lightColor[ii];
        float       adp = abs(dot(norm,ld));        // abs allows 2-sided rendering (when backface culling is off)
        ret += alb * lc * adp;                      // Diffuse term
        float4      halfAngle = normalize(float4(ld[0],ld[1],ld[2]-1,0));
        float4      del = halfAngle - norm;
        float       d2 = dot(del,del);
        float4      spec = specularMap.Sample(sstate,frag.uv);
        spec[3] = 0;                                // ignore specular alpha term (meaningless)
        ret = saturate(ret + spec * lc * exp(-d2*128)); // Specular term
    }
    return ret;     // Resulting value is automatically clipped to [0,1]
}
