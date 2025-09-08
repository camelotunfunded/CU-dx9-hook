#include <d3dx9.h>

// Pointeur vers l'effet de shader pour la végétation / Pointer to the vegetation shader effect
LPD3DXEFFECT g_vegetationEffect = nullptr;
// Handles pour accéder aux paramètres du shader / Handles to access shader parameters
D3DXHANDLE g_timeHandle = nullptr;
D3DXHANDLE g_wvpHandle = nullptr;
D3DXHANDLE g_baseTextureHandle = nullptr;

// Indique si la texture de végétation est active / Indicates if the vegetation texture is active
bool g_isVegetationTextureActive = false;

// Structure représentant un sommet simple avec position et coordonnées de texture
// Structure representing a simple vertex with position and texture coordinates
struct SimpleVertex
{
    float x, y, z; // Position du sommet / Vertex position
    float u, v;    // Coordonnées de texture / Texture coordinates
};

// Charge le shader de végétation depuis un fichier et récupère les handles des paramètres
// Loads the vegetation shader from file and retrieves parameter handles
void LoadVegetationShader(IDirect3DDevice9* device)
{
    ID3DXBuffer* errorBuffer = nullptr;

    // Création de l'effet à partir du fichier vegetation.fx
    // Create the effect from the vegetation.fx file
    HRESULT hr = D3DXCreateEffectFromFileA(
        device,
        "C:\\D3D9Proxy\\shaders\\vegetation.fx",
        nullptr, nullptr, D3DXSHADER_DEBUG, nullptr,
        &g_vegetationEffect,
        &errorBuffer
    );

    // Vérifie si le chargement a échoué / Check if loading failed
    if (FAILED(hr)) {
        if (errorBuffer) {
            // Affiche le message d'erreur du shader / Output shader error message
            OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
            errorBuffer->Release();
        }
        else {
            // Affiche un message d'erreur générique / Output generic error message
            OutputDebugStringA("[ERROR] Failed to load vegetation.fx (no errorBuffer)\n");
        }
        return;
    }

    // Récupère les handles des paramètres du shader / Retrieve shader parameter handles
    g_timeHandle = g_vegetationEffect->GetParameterByName(nullptr, "time");
    g_wvpHandle = g_vegetationEffect->GetParameterByName(nullptr, "WorldViewProjection");
    g_baseTextureHandle = g_vegetationEffect->GetParameterByName(nullptr, "baseTexture");

    // Vérifie si chaque handle a été trouvé / Check if each handle was found
    if (!g_timeHandle)         OutputDebugStringA("[WARN] Shader handle 'time' not found\n");
    if (!g_wvpHandle)          OutputDebugStringA("[WARN] Shader handle 'WorldViewProjection' not found\n");
    if (!g_baseTextureHandle)  OutputDebugStringA("[WARN] Shader handle 'baseTexture' not found\n");

    // Affiche un message de succès / Output success message
    OutputDebugStringA("[OK] vegetation.fx loaded successfully\n");
}
