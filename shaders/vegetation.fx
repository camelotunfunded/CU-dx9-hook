texture baseTexture;

sampler baseSampler = sampler_state {
    Texture = <baseTexture>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;
    AddressU = WRAP;
    AddressV = WRAP;
};

float4x4 WorldViewProjection;
float time;

struct VS_INPUT_252 {
    float3 pos : POSITION;
    float3 normal : NORMAL;     // ignorée mais nécessaire pour FVF 0x252
    float2 tex : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 pos : POSITION;
    float2 tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT_252 input) {
    VS_OUTPUT output;

    float4 localPos = float4(input.pos, 1.0);
    localPos.x += sin(input.pos.x * 9.0 + time) * 0.9; // vent

    output.pos = mul(localPos, WorldViewProjection);
    output.tex = input.tex;

    return output;
}

float4 PS(VS_OUTPUT input) : COLOR {
    float4 texColor = tex2D(baseSampler, saturate(input.tex));

    // Sécurité alpha minimale
    if (texColor.a < 0.05)
        texColor.a = 0.05;

    // Remplace le vert de debug par une couleur par défaut si besoin
    if (all(texColor.rgb == float3(0, 1, 0)))
    texColor.rgb = float3(88.0/255.0, 134.0/255.0, 69.0/255.0);


    return texColor;
}

technique Wind{
    pass P0 {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}
