#include "dx11_shared.hlsl"

RWTexture2D<unorm float4>  BackBuffer      : register(u0);
Texture2D<uint>            HeadPointersSRV : register(t0);
StructuredBuffer<ListNode> LinkedListSRV   : register(t1);

static const uint FRAGMENT_COUNT    = 32;
static const uint MSAA_SAMPLE_COUNT = 4;

struct ListSubNode {
    float depth;
    uint  color;
};


[numthreads(8, 8, 1)]
void CSResolve(uint3 id : SV_DispatchThreadID) {
    
    float4 backBuffer = BackBuffer[id.xy];
    float4 resolveBuffer = float4(0.0, 0.0, 0.0, 0.0f);
    
    ListSubNode nodes[FRAGMENT_COUNT];   
    for (uint sampleIdx = 0; sampleIdx < MSAA_SAMPLE_COUNT; sampleIdx++) {
       
        uint count = 0;
        uint nodeIdx = HeadPointersSRV[id.xy];
        if (nodeIdx == 0xFFFFFFFF)
            return;
            
        while (nodeIdx != 0xFFFFFFFF && count < FRAGMENT_COUNT) {
            ListNode node = LinkedListSRV[nodeIdx];
            if (node.coverage & (1 << sampleIdx)) {
                nodes[count].depth = asfloat(node.depth);
                nodes[count].color = node.color;
                count++;
            }
            nodeIdx = node.next;
        }
              
        for (uint i = 1; i < count; i++) {
            ListSubNode t = nodes[i];
            uint j = i;
            while (j > 0 && (nodes[j - 1].depth < t.depth)) {
                nodes[j] = nodes[j - 1];
                j--;
            }
            nodes[j] = t;
        }
         
        float4 dstPixelColor = backBuffer;
        for (uint index = 0; index < count; index++) {
            float4 srcPixelColor = unpackColor(nodes[index].color);
            dstPixelColor = lerp(dstPixelColor, srcPixelColor, srcPixelColor.a);
        }
        resolveBuffer += dstPixelColor;
    }
   
    BackBuffer[id.xy] = resolveBuffer / MSAA_SAMPLE_COUNT;
}
