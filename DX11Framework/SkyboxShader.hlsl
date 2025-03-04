cbuffer ConstantBuffer : register(b0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
}

TextureCube skybox : register(t0);
SamplerState bilinearSampler : register(s0);

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 TexCoord : TEXCOORD;
};

VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD)
{
    VS_Out output = (VS_Out) 0;

    float3 positionTranslated = Position;
    float4 Pos4 = float4(positionTranslated, 1.0f);
    output.position = mul(Pos4, World);

    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.TexCoord = Position;
    return output;
}

float4 PS_main(VS_Out input) : SV_TARGET
{
    return skybox.Sample(bilinearSampler, input.TexCoord);
}