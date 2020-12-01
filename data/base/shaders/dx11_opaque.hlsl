
#include "dx11_shared.hlsl"

float4
PSOpaque(Frag frag)
    : SV_Target     // A "system-value semantic" specifying output be stored in a render target
{
    float3      albedo = albedoMap.Sample(sstate,frag.uv).rgb;
    float3      mod = modulationMap.Sample(sstate,frag.uv).rgb;
    float3      ma = albedo * mod * (255.05f/64.0f);
    float3      color = calcColor(frag,ma);
    return float4(color,Scene.ambientColor.a);
}
