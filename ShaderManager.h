#pragma once
#include <d3dx9.h>

extern LPD3DXEFFECT g_vegetationEffect;
extern D3DXHANDLE g_timeHandle;
extern D3DXHANDLE g_wvpHandle;
extern D3DXHANDLE g_baseTextureHandle;
// ShaderManager.h
extern bool g_isVegetationTextureActive;

void LoadVegetationShader(IDirect3DDevice9* device);
