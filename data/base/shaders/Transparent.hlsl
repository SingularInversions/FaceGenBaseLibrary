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

 
    float3 N = normalize(input.Normal.xyz); 

    float3 color = albedo.xyz * FrameBuffer.AmbientColor.xyz;

    for (uint index = 0; index < 2; index++) {
        float3 L = FrameBuffer.LightDirection[index].xyz;
        float3 R = reflect(-L, N);
        float3 kD = albedo.rgb * FrameBuffer.LightColor[index].rgb * abs(dot(N, L));
        float3 kS = specular.rgb * FrameBuffer.LightColor[index].rgb * pow(max(dot(float3(0.0f, 0.0, 1.0), R), 0.0), 128);
        color += (kD + kS);
    }

    return float4(color, albedo.a);

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

Texture2D<float4> TextureBackground : register(t0);


void VertexShaderTransparentSecondPass(uint vertexID : SV_VertexID, out float4 position : SV_Position, out float2 texcoord: TEXCOORD) {
	texcoord = float2((vertexID << 1) & 2, vertexID & 2);
	position = float4(texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 1.0f, 1.0f);
}

float4 PixelShaderTransparentSecondPass(float4 position : SV_Position, float2 texcoord : TEXCOORD, out float depth : SV_Depth) : SV_TARGET0
{

    const uint FRAGMENT_COUNT = 32;

    ListNode nodes[FRAGMENT_COUNT];
	
	uint count = 0;
    uint idx  = HeadPointersUAV[uint2(position.xy)];
	
    [loop]
    while (idx != 0xFFFFFFFF && count < FRAGMENT_COUNT) {
        nodes[count] = LinkedListUAV[idx];
		idx = nodes[count].Next;
		count++;
       // if (count >= FRAGMENT_COUNT)
       //     return float4(1.0, 0.0, 0.0, 1.0);
    }
	
   
    [loop]
    for (uint i = 1u; i < count; i++) {
        uint j = i;
        while (j > 0 && nodes[j - 1].Depth < nodes[j + 0].Depth) {
            ListNode t  = nodes[j];
            nodes[j + 0] = nodes[j - 1];
            nodes[j - 1] = t;
            j--;
        }
    }
	
    float4 color = TextureBackground.Sample(LinearSamplerState, texcoord);
    for (uint index = 0u; index < count; index++) {
        float4 t = UnpackColor(nodes[index].Color);
        color = lerp(color, t, t.a);
    }
    
    
    //Try z buffer transpancety
    depth = count > 0 ? nodes[count - 1].Depth : 1.0f;
    
	return color;
}
