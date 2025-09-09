// === Vertex Format (herbe) ===
// position : float3 (POSITION)
// color    : float4 (COLOR0)
// texcoord : float2 (TEXCOORD0)

float4x4 WorldViewProjection;  // Matrice WVP
float    Time;                 // Temps en secondes
texture  BaseTexture;          // Texture d'herbe (bind via SetTexture)

sampler2D BaseSampler = sampler_state
{
    Texture = <BaseTexture>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR0;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color    : COLOR0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    // --- mouvement exagéré ---
    // onde rapide et forte
    float sway = sin(input.position.x * 0.5 + Time * 5.0) * 2.0
               + cos(input.position.z * 0.7 + Time * 3.0) * 1.0;

    float3 animatedPos = input.position;
    animatedPos.y += sway; // grand déplacement vertical

    output.position = mul(float4(animatedPos, 1.0), WorldViewProjection);
    output.texcoord = input.texcoord;
    output.color    = input.color;

    return output;
}

float4 PS(VS_OUTPUT input) : COLOR
{
    float4 texColor = tex2D(BaseSampler, input.texcoord);
    return texColor * input.color; // applique alpha/couleur diffuse
}

technique Vegetation
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}
