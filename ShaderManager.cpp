#include <d3dx9.h>

LPD3DXEFFECT g_vegetationEffect = nullptr;
D3DXHANDLE g_timeHandle = nullptr;
D3DXHANDLE g_wvpHandle = nullptr;
D3DXHANDLE g_baseTextureHandle = nullptr;

bool g_isVegetationTextureActive = false;
struct SimpleVertex
{
    float x, y, z;
    float u, v;
};

void LoadVegetationShader(IDirect3DDevice9* device)
{
    ID3DXBuffer* errorBuffer = nullptr;

    HRESULT hr = D3DXCreateEffectFromFileA(
        device,
        "C:\\D3D9Proxy\\shaders\\vegetation.fx",
        nullptr, nullptr, D3DXSHADER_DEBUG, nullptr,
        &g_vegetationEffect,
        &errorBuffer
    );

    if (FAILED(hr)) {
        if (errorBuffer) {
            OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
            errorBuffer->Release();
        }
        else {
            OutputDebugStringA("[ERROR] Failed to load vegetation.fx (no errorBuffer)\n");
        }
        return;
    }

    g_timeHandle = g_vegetationEffect->GetParameterByName(nullptr, "time");
    g_wvpHandle = g_vegetationEffect->GetParameterByName(nullptr, "WorldViewProjection");
    g_baseTextureHandle = g_vegetationEffect->GetParameterByName(nullptr, "baseTexture");

    if (!g_timeHandle)         OutputDebugStringA("[WARN] Shader handle 'time' not found\n");
    if (!g_wvpHandle)          OutputDebugStringA("[WARN] Shader handle 'WorldViewProjection' not found\n");
    if (!g_baseTextureHandle)  OutputDebugStringA("[WARN] Shader handle 'baseTexture' not found\n");

    OutputDebugStringA("[OK] vegetation.fx loaded successfully\n");
}
