// Param�tres contr�lables depuis le C++ (si vous le souhaitez plus tard)
float BloomThreshold = 0.1f;
float BloomMagnitude = 10.0f;

// Taille d'un pixel (Texel), calcul�e et envoy�e par le C++
float2 TexelSize;

// Textures re�ues du C++
texture sceneTex; // Pass 0: Image originale
texture blurTex;  // Pass 1: Image apr�s le premier flou

// Samplers pour lire les textures
sampler2D sceneSampler = sampler_state
{
    Texture = <sceneTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler2D blurSampler = sampler_state
{
    Texture = <blurTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};


// Vertex Shader simple pour dessiner un carr� en plein �cran
void VS(in float4 Pos : POSITION, in float2 Tex : TEXCOORD0,
        out float4 oPos : POSITION, out float2 oTex : TEXCOORD0)
{
    oPos = Pos;
    oTex = Tex;
}

// --- PASS 0: Extrait les hautes lumi�res et applique un flou horizontal ---
float4 PS_BrightPassAndBlurH(float2 Tex : TEXCOORD0) : COLOR0
{
    float4 totalColor = float4(0, 0, 0, 0);

    // 1. Extraire les zones plus lumineuses que le seuil (Threshold)
    float4 originalColor = tex2D(sceneSampler, Tex);
    float brightness = dot(originalColor.rgb, float3(0.299, 0.587, 0.114));
    float4 brightColor = originalColor * saturate((brightness - BloomThreshold) / (1.0 - BloomThreshold));

    // 2. Appliquer un flou Gaussien horizontal
    float weights[7] = { 0.05, 0.1, 0.2, 0.3, 0.2, 0.1, 0.05 };
    float2 texelOffset = float2(TexelSize.x, 0);

    for (int i = -3; i <= 3; i++)
    {
        totalColor += tex2D(sceneSampler, Tex + i * texelOffset) * weights[i+3];
    }
    
    // On ne garde que les parties lumineuses du flou
    return totalColor * brightColor;
}


// --- PASS 1: Applique un flou vertical et combine ---
float4 PS_BlurVAndCombine(float2 Tex : TEXCOORD0) : COLOR0
{
    float4 totalColor = float4(0, 0, 0, 0);

    // Appliquer un flou Gaussien vertical sur le r�sultat de la passe pr�c�dente
    float weights[7] = { 0.05, 0.1, 0.2, 0.3, 0.2, 0.1, 0.05 };
    float2 texelOffset = float2(0, TexelSize.y);

    for (int i = -3; i <= 3; i++)
    {
        totalColor += tex2D(blurSampler, Tex + i * texelOffset) * weights[i+3];
    }

    // On retourne la couleur du bloom, intensifi�e par la magnitude
    return totalColor * BloomMagnitude;
}


// --- Technique utilis�e par le C++ ---
technique Bloom
{
    pass p0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_BrightPassAndBlurH();
    }
    pass p1
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS_BlurVAndCombine();
    }
}