// Leaves.fx

float time;
matrix WorldViewProjection;

texture baseTexture;
sampler baseSampler = sampler_state {
    Texture = <baseTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};

struct VS_IN {
    float4 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct VS_OUT {
    float4 pos : POSITION;
    float2 tex : TEXCOORD0;
};

VS_OUT vs_main(VS_IN input)
{
    VS_OUT output;
    float windOffset = sin(time * 2.0 + input.pos.x * 0.1) * 0.05;
    input.pos.x += windOffset;
    output.pos = mul(input.pos, WorldViewProjection);
    output.tex = input.tex;
    return output;
}

float4 ps_main(VS_OUT input) : COLOR
{
    return tex2D(baseSampler, input.tex);
}

technique Wind
{
    pass P0
    {
        VertexShader = compile vs_3_0 vs_main();
        PixelShader  = compile ps_3_0 ps_main();
    }
}
