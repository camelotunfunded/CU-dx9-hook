#include <d3dx9.h>

// Pointeur vers l'effet de shader pour la v�g�tation / Pointer to the vegetation shader effect
LPD3DXEFFECT g_vegetationEffect = nullptr;
// Handles pour acc�der aux param�tres du shader / Handles to access shader parameters
D3DXHANDLE g_timeHandle = nullptr;
D3DXHANDLE g_wvpHandle = nullptr;
D3DXHANDLE g_baseTextureHandle = nullptr;

// Indique si la texture de v�g�tation est active / Indicates if the vegetation texture is active
bool g_isVegetationTextureActive = false;

// Structure repr�sentant un sommet simple avec position et coordonn�es de texture
// Structure representing a simple vertex with position and texture coordinates
struct SimpleVertex
{
    float x, y, z; // Position du sommet / Vertex position
    float u, v;    // Coordonn�es de texture / Texture coordinates
};

// Charge le shader de v�g�tation depuis un fichier et r�cup�re les handles des param�tres
// Loads the vegetation shader from file and retrieves parameter handles
void LoadVegetationShader(IDirect3DDevice9* device)
{
    ID3DXBuffer* errorBuffer = nullptr;

    // Cr�ation de l'effet � partir du fichier vegetation.fx
    // Create the effect from the vegetation.fx file
    HRESULT hr = D3DXCreateEffectFromFileA(
        device,
        "C:\\D3D9Proxy\\shaders\\vegetation.fx",
        nullptr, nullptr, D3DXSHADER_DEBUG, nullptr,
        &g_vegetationEffect,
        &errorBuffer
    );

    // V�rifie si le chargement a �chou� / Check if loading failed
    if (FAILED(hr)) {
        if (errorBuffer) {
            // Affiche le message d'erreur du shader / Output shader error message
            OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
            errorBuffer->Release();
        }
        else {
            // Affiche un message d'erreur g�n�rique / Output generic error message
            OutputDebugStringA("[ERROR] Failed to load vegetation.fx (no errorBuffer)\n");
        }
        return;
    }

    // R�cup�re les handles des param�tres du shader / Retrieve shader parameter handles
    g_timeHandle = g_vegetationEffect->GetParameterByName(nullptr, "time");
    g_wvpHandle = g_vegetationEffect->GetParameterByName(nullptr, "WorldViewProjection");
    g_baseTextureHandle = g_vegetationEffect->GetParameterByName(nullptr, "baseTexture");

    // V�rifie si chaque handle a �t� trouv� / Check if each handle was found
    if (!g_timeHandle)         OutputDebugStringA("[WARN] Shader handle 'time' not found\n");
    if (!g_wvpHandle)          OutputDebugStringA("[WARN] Shader handle 'WorldViewProjection' not found\n");
    if (!g_baseTextureHandle)  OutputDebugStringA("[WARN] Shader handle 'baseTexture' not found\n");

    // Affiche un message de succ�s / Output success message
    OutputDebugStringA("[OK] vegetation.fx loaded successfully\n");
}
