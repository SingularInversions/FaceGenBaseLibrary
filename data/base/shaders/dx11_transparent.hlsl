
#include "dx11_shared.hlsl"

uint
packColor(float4 color)
{
    return
        (uint(color.r * 255.99f) << 24) |
        (uint(color.g * 255.99f) << 16) |
        (uint(color.b * 255.99f) << 8) |
         uint(color.a * 255.99f);
}

float4
unpackColor(uint color)
{
    float4 result;
    result.r = float((color >> 24) & 0x000000FF) / 255.0f;
    result.g = float((color >> 16) & 0x000000FF) / 255.0f;
    result.b = float((color >> 8)  & 0x000000FF) / 255.0f;
    result.a = float(color & 0x000000FF)         / 255.0f;
    return saturate(result);
}

struct ListNode
{
    uint        next;
    uint        color;
    float       depth;
};

RWTexture2D<uint>            HeadPointersUAV;

globallycoherent        // Sync memory across entire GPU:
RWStructuredBuffer<ListNode> LinkedListUAV;

[earlydepthstencil]     // Only run this function on fragments that pass depth test
float4
PSTransparentPass1(Frag frag) : SV_TARGET0
{
    float4      albedo = albedoMap.Sample(sstate,frag.uv);
    if (albedo.a > 0.01f) {     // Discard mostly transparent (value varies continuously due to sampling)
        uint        nodeIdx = LinkedListUAV.IncrementCounter();     // atomic index counter
        uint        prevHead;
        // Atomically set pixel to 'nodeIdx' and get back 'prevHead':
        InterlockedExchange(HeadPointersUAV[uint2(frag.pos.xy)],nodeIdx,prevHead);
        ListNode    node;
        float3      color = calcColor(frag,albedo.rgb);
        node.color = packColor(float4(color,albedo.a)); 
        node.depth = frag.pos.z;
        node.next  = prevHead;
        LinkedListUAV[nodeIdx] = node;
    }
    return float4(0,0,0,0);
}

float4
VSTransparentPass2(
    // SV_VertexID is a "system-value symantic" that simply passes sequential index numbers from the
    // Draw() call to the vertex shader. Ie. Draw(3,0) invokes this with values 0,1,2:
    uint            idx : SV_VertexID)
    : SV_POSITION   // Return value semantic
{
    // Generates triangle that covers the entire clip space (XY in [-1,1])
    // A single clipped triangle is apparently better than a quad.
    float4      pos;
    pos.x = float(idx/2)*4.0f-1.0f;     // -1, -1,  3
    pos.y = float(idx%2)*4.0f-1.0f;     // -1,  3, -1
    pos.z = 0.0f;
    pos.w = 1.0f;
    return pos;
}

struct  Node
{
    uint    color;
    float   depth;
};

float4
PSTransparentPass2(float4 pos : SV_POSITION)
    : SV_TARGET0
{
    // Copy linked list into local memory contiguous list for faster sorting:
    const uint      NODES_MAX_SIZE = 32;    // Don't increase to 64, crashes at least 1 AMD GPU
    Node            nodes[NODES_MAX_SIZE];
    uint            numNodes = 0;
    uint            nodeIdx  = HeadPointersUAV[uint2(pos.xy)];
    [loop]
    while (nodeIdx != 0xFFFFFFFF && numNodes < NODES_MAX_SIZE) {
        ListNode        listNode = LinkedListUAV[nodeIdx];
        nodes[numNodes].color = listNode.color;
        nodes[numNodes].depth = listNode.depth;
        nodeIdx = listNode.next;
        ++numNodes;
    }
    // Insertion sort - largest to smallest depth:
    [loop]
    for (uint ii=1; ii<numNodes; ii++) {
        uint        jj = ii;
        while ((jj > 0) && (nodes[jj-1].depth < nodes[jj].depth)) {
            Node        t  = nodes[jj];
            nodes[jj] = nodes[jj-1];
            nodes[jj-1] = t;
            --jj;
        }
    }
    float4          ret = float4(0,0,0,0);
    for (uint nn=0; nn<numNodes; ++nn) {
        uint            mm = numNodes-nn-1;     // Front to back
        float4          clr = unpackColor(nodes[mm].color);
        ret = lerp(clr,ret,ret.a);
        ret.a = ret.a + (1.0f-ret.a)*clr.a;
    }
    return ret;
}
