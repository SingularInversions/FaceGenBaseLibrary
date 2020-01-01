
#include "dx11_shared.hlsl"

float4
PSOpaque(Frag frag)
    : SV_Target     // A "system-value semantic" specifying output be stored in a render target
{
    float4      albedo = albedoMap.Sample(sstate,frag.uv);
    float3      color = calcColor(frag,albedo.rgb);
    return float4(color,Scene.ambientColor.a);
}
