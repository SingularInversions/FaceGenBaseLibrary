
#include "dx11_shared.hlsl"




globallycoherent RWTexture2D<uint> HeadPointersUAV          : register(u0);
globallycoherent RWStructuredBuffer<ListNode> LinkedListUAV : register(u1);

[earlydepthstencil]     
void PSTransparent(Frag frag, uint coverage : SV_Coverage) {
    
    float4  albedo = albedoMap.Sample(sstate, frag.uv);
    if (albedo.a > 0.01f) {    
        
        uint nodeIdx = LinkedListUAV.IncrementCounter();     
        uint prevHead;
   
        InterlockedExchange(HeadPointersUAV[uint2(frag.pos.xy)], nodeIdx, prevHead);
        ListNode node;
  
        node.color = packColor(float4(calcColor(frag, albedo.rgb), albedo.a));
        node.depth = asuint(frag.pos.z);
        node.next  = prevHead;
        node.coverage = coverage;
        
        LinkedListUAV[nodeIdx] = node;
    }
  
}

