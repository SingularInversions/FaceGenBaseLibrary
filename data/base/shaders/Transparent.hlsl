#include "Common.hlsl"

Texture2D<float4> TextureAlbedo   : register(t0);
Texture2D<float4> TextureSpecular : register(t1);

SamplerState LinearSamplerState : register(s0);


struct ListNode {
    uint   Next;
	uint   Color;
	float  Depth;
};


struct InputVS {
	float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Texcoord : TEXCOORD;
};

struct InputPS {
    float4 Position : SV_POSITION; 
    float4 Normal   : NORMAL; 
    float2 Texcoord : TEXCOORD;
};


globallycoherent RWTexture2D<uint>            HeadPointersUAV;
globallycoherent RWStructuredBuffer<ListNode> LinkedListUAV;


float4 ComputeLight(InputPS input)
{


    float4 albedo   = TextureAlbedo.Sample(LinearSamplerState, input.Texcoord);
    float4 specular = TextureSpecular.Sample(LinearSamplerState, input.Texcoord);

 
    float4 N = normalize(input.Normal); 

    float4 result = float4(0.0, 0.0, 0.0f, 0.0f);

    for (uint index = 0; index < 2; index++) {
        float4 L = FrameBuffer.LightDirection[index];
        float4 diffuse = albedo * FrameBuffer.LightColor[index] * abs(dot(N, L));
        result += diffuse;
    }

    return result;

}



InputPS VertexShaderTransparentFirstPass(InputVS input) {

    InputPS result;
    result.Position = mul(FrameBuffer.WorldViewMatrix, float4(input.Position, 1.0f));
    result.Position = mul(FrameBuffer.ProjectMatrix, result.Position);
    result.Normal   = normalize(mul(FrameBuffer.WorldViewMatrix, float4(input.Normal, 0.0f)));
    result.Texcoord = input.Texcoord;
    return result;

}

[earlydepthstencil]
float4 PixelShaderTransparentFirstPass(InputPS input) : SV_TARGET0
{

	uint nodeIdx = LinkedListUAV.IncrementCounter();

	uint prevHead;
    InterlockedExchange(HeadPointersUAV[uint2(input.Position.xy)], nodeIdx, prevHead);

    ListNode node;
    node.Color = PackColor(ComputeLight(input)); 
    node.Depth = input.Position.z;
    node.Next  = prevHead;

  

    LinkedListUAV[nodeIdx] = node;
	
    return float4(0.0, 0.0, 0.0, 0.0);
}




void VertexShaderTransparentSecondPass(uint vertexID : SV_VertexID, out float4 position : SV_Position, out float2 texcoord: TEXCOORD) {
	texcoord = float2((vertexID << 1) & 2, vertexID & 2);
	position = float4(texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
}

float4 PixelShaderTransparentSecondPass(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_TARGET0 {

    const uint FRAGMENT_COUNT = 32;

    ListNode nodes[FRAGMENT_COUNT];
	
	uint count = 0;
    uint idx  = HeadPointersUAV[uint2(position.xy)];
	
  
    while (idx != 0xFFFFFFFF && count < FRAGMENT_COUNT) {
        nodes[count] = LinkedListUAV[idx];
		idx = nodes[count].Next;
		count++;
	}
	
  
    for (uint i = 1u; i < count; i++) {
        uint j = i;
        while (j > 0 && nodes[j - 1].Depth < nodes[j + 0].Depth) {
            ListNode t  = nodes[j];
            nodes[j + 0] = nodes[j - 1];
            nodes[j - 1] = t;
            j--;
        }
    }
	
    float4 color = float4(0.5, 0.5, 0.5, 1.0);
    for (uint index = 0u; index < count; index++) {
        float4 t = UnpackColor(nodes[index].Color);
        color = lerp(color, t, t.a);
    }
       
		
	return color;
}
