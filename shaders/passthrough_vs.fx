// Ce vertex shader ne fait rien d'autre que de passer les coordonnées du sommet
// directement au rendu, garantissant un affichage 2D correct.
void main(in float4 position : POSITION,
          in float2 texCoord : TEXCOORD0,
          out float4 oPosition : POSITION,
          out float2 oTexCoord : TEXCOORD0)
{
    oPosition = position;
    oTexCoord = texCoord;
}