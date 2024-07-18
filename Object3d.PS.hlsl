#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
};

//平行光源
struct DirectionalLight
{
    float4 color; //!< ライトの色
    float3 direction; //ライトの向き
    float intensity; //輝度
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

//ピクセルシェーダーの出力
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

//ピクセルシェーダー
PixelShaderOutput main(VertexShaderOutput input)
{
    //TextureをSamplingする
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    
    PixelShaderOutput output;
    
    //Lightingする場合
    if (gMaterial.enableLighting != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
       
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }
    else
    {
        //Lightingしない場合、前回までと同じ演算
        output.color = gMaterial.color * textureColor;
    }
    return output;
}
