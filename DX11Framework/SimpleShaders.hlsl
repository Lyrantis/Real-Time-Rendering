struct PositionalLight
{
    float3 Position;
    bool enabled;
    float4 Colour;
    float3 Direction;
    float MaxDistance;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
    float4 DiffuseLight;
    float3 LightDir;
    float count;
    float4 AmbientLight;
    float4 specularLight;
    float4 specularMaterial;
    float3 cameraPosition;
    float specPower;
    PositionalLight Lights[16];
    uint hasTexture;
    uint hasSpecMap;
}

Texture2D diffuseTex : register(t0);
Texture2D specMap : register(t1);
SamplerState bilinearSampler : register(s0);

struct VS_Out
{
    float4 position : SV_POSITION;
    float3 PosW : POSITION0;
    float4 Colour : COLOUR;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD)
{   
    VS_Out output = (VS_Out)0;

    float4 Pos4 = float4(Position, 1.0f);
   // Pos4.y += sin(count) * 2;
    output.position = mul(Pos4, World);
    output.PosW = output.position;

    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    float4 nor4 = float4(Normal, 0.0f);
    output.Normal = mul(nor4, World);
    //output.Normal = normalize(output.Normal);
    
    output.TexCoord = TexCoord;
    return output;
}

float4 PS_main(VS_Out input) : SV_TARGET
{  
    //World Lighting
    float4 normal = float4(input.Normal, 1.0f);
    float4 NormalW = normalize(normal);
    float4 texColour = diffuseTex.Sample(bilinearSampler, input.TexCoord);
    clip(texColour.a - 0.1);
    
    float diffuseIntensity = saturate(dot((float3) NormalW, -LightDir));
    float4 diffTotal = diffuseIntensity * DiffuseLight * texColour;
    
    float4 ambientTotal = AmbientLight * texColour;
    
    float3 reflectedLight = reflect(normalize(LightDir), normalize(input.Normal));
    float3 dirToCamera = normalize(cameraPosition - input.PosW);
    float specIntensity = saturate(dot(reflectedLight, dirToCamera));
    specIntensity = pow(specIntensity, specPower);
    float4 specTotal = specIntensity * specularLight;
    if (hasSpecMap) 
    {
        specTotal *= specMap.Sample(bilinearSampler, input.TexCoord);
    } 
    else
    {
        specTotal *= specularMaterial;
    }
    
    float4 overallLight = diffTotal + ambientTotal + specTotal;
    for (int i = 0; i < 16; i++)
    {
        if (Lights[i].enabled && (distance(Lights[i].Position, input.PosW.xyz) < Lights[i].MaxDistance))
        {
            float diffuseIntensity = saturate(dot(NormalW.xyz, -Lights[i].Direction));
            float4 diffuseTotal = diffuseIntensity * Lights[i].Colour * texColour;
    
            float3 reflectedLight = reflect(normalize(Lights[i].Direction), normalize(input.Normal));
            float3 dirToCamera = normalize(cameraPosition - input.PosW);
            float specIntensity = saturate(dot(reflectedLight, dirToCamera));
            specIntensity = pow(specIntensity, specPower);
            float4 specularTotal = specIntensity * Lights[i].Colour;
        
            if (hasSpecMap)
            {
                specularTotal *= specMap.Sample(bilinearSampler, input.TexCoord);
            }
            else
            {
                specularTotal *= specularMaterial;
            }

            overallLight += diffuseTotal + specularTotal;
        }
        
    }
    return overallLight;
}