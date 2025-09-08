#include <sstream>
#define NOMINMAX // pour éviter les conflits avec min/max définis dans Windows.h // Prevent conflicts with min/max macros from Windows.h
#include <cstdint>      // uint32_t, uint8_t
#include <cstddef>      // size_t
#include <algorithm>    // std::min
#include "TextureHash.h"
#include "TextureLogger.h"
#include "IDirect3DDevice9Proxy.h"
#include <d3dx9.h>
// Pour stocker chaque texture de végétation par hash (et gérer leur durée de vie COM)
// Used to store each vegetation texture by hash (and manage COM lifetimes)
#include <map>
static std::map<uint32_t, IDirect3DTexture9*> g_vegetationTexturesByHash;


//COMPTEUR POUR ENDSCENE
// COUNTER FOR ENDSCENE
static int endSceneCallCount = 0;


// --- VARIABLES POUR LA DÉTECTION "IN-GAME" ---
// --- VARIABLES FOR "IN-GAME" DETECTION ---
static int drawCallCount = 0;
static bool isInGame = false;

// --- RESSOURCES POUR L'OISEAU ---
// --- RESOURCES FOR THE BIRD SPRITE ---
//BIRD STOPED for now
LPDIRECT3DTEXTURE9 g_pBirdTexture = nullptr;
IDirect3DVertexBuffer9* g_pBirdVB = nullptr;


#include <filesystem>
#include "ShaderManager.h"
#include "KnownVegetation.h" // ou le bon header si les hashes sont stockés ailleurs // Or the correct header if hashes are defined elsewhere

#include <fstream>


struct SimpleVertex
{
	float x, y, z;
	float u, v;
};


// temporaire pour l'export JPG des frames
// Temporary JPG export utility for captured frames
void ExportBackBufferAsJPG(IDirect3DDevice9* device, int frameIndex)
{
	IDirect3DSurface9* pBackBuffer = nullptr;
	if (SUCCEEDED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		char filename[256];
		sprintf_s(filename, "C:\\D3D9Proxy\\frames\\frame_%03d.jpg", frameIndex);

		// Exporte en JPG (qualité = 90)
		D3DXSaveSurfaceToFileA(filename, D3DXIFF_JPG, pBackBuffer, NULL, NULL);

		pBackBuffer->Release();
	}
}


IDirect3DDevice9Proxy* IDirect3DDevice9Proxy::lastDevice = NULL;

void* IDirect3DDevice9Proxy::callbacks[D3D9_DEVICE_FUNC_COUNT] = { NULL };

extern std::set<uint32_t> knownVegetationHashes;

// Ajout des déclarations extern pour les variables globales utilisées dans BeginScene/EndScene
extern bool g_triggerNextFrameCapture;
extern bool g_captureNextFrame;

extern int g_frameIndex;
extern LPD3DXEFFECT g_vegetationEffect;
extern D3DXHANDLE g_timeHandle;

//void InitBirdResources(IDirect3DDevice9* device)
//{
//	// On ne charge les ressources qu'une seule fois // Only load bird resources once
//	if (g_pBirdTexture == nullptr)
//	{
//		// Charge la texture de l'oiseau depuis un fichier
//		HRESULT hr = D3DXCreateTextureFromFileA(device, "C:\\D3D9Proxy\\textures\\bird.png", &g_pBirdTexture);
//
//		// --- VÉRIFICATION D'ERREUR ---
//		if (FAILED(hr))
//		{
//			MessageBoxA(NULL, "Échec du chargement de la texture pour l'oiseau.\n\nVérifiez que ce fichier existe :\nC:\\D3D9Proxy\\textures\\bird.png", "Erreur Texture Oiseau", MB_OK | MB_ICONERROR);
//		}
//	}
//
//	if (g_pBirdVB == nullptr)
//	{
//		// Crée le Vertex Buffer pour notre carré (sprite)
//		device->CreateVertexBuffer(4 * (sizeof(float) * 5), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_TEX1, D3DPOOL_DEFAULT, &g_pBirdVB, NULL);
//
//		struct BirdVertex { float x, y, z, u, v; };
//		BirdVertex* vertices;
//
//		g_pBirdVB->Lock(0, 0, (void**)&vertices, 0);
//		// On définit un carré de 10x10 unités
//		vertices[0] = { -5.0f,  5.0f, 0.0f, 0.0f, 0.0f }; // Haut-gauche
//		vertices[1] = { 5.0f,  5.0f, 0.0f, 1.0f, 0.0f }; // Haut-droit
//		vertices[2] = { -5.0f, -5.0f, 0.0f, 0.0f, 1.0f }; // Bas-gauche
//		vertices[3] = { 5.0f, -5.0f, 0.0f, 1.0f, 1.0f }; // Bas-droit
//		g_pBirdVB->Unlock();
//	}
//}


extern "C" UINT WINAPI D3D9DeviceFuncHook(UINT funcId, void* funcRef) {
	if (!IDirect3DDevice9Proxy::lastDevice)
		return D3D_DEVICE_PROXY_STATUS_NOTREADY;
	if (funcId > (D3D9_DEVICE_FUNC_COUNT - 1))
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC_ID;
	if (!funcRef)
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC;
	IDirect3DDevice9Proxy::callbacks[funcId] = funcRef;
	return 1;
}

extern "C" UINT WINAPI D3D9DeviceFuncUnHook(UINT funcId) {
	if (!IDirect3DDevice9Proxy::lastDevice)
		return D3D_DEVICE_PROXY_STATUS_NOTREADY;
	if (funcId > (D3D9_DEVICE_FUNC_COUNT - 1))
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC_ID;
	IDirect3DDevice9Proxy::callbacks[funcId] = NULL;
	return 1;
}


IDirect3DDevice9Proxy::IDirect3DDevice9Proxy(IDirect3DDevice9* pOriginal) {
	origIDirect3DDevice9 = pOriginal;
	lastDevice = this;
	// On initialise les ressources de l'oiseau ici.
	// Initialize bird resources here.

	// C'est le meilleur endroit car on est sûr que le device existe.
	 // Best place to do it, since we are sure the device exists.
}

IDirect3DDevice9Proxy::~IDirect3DDevice9Proxy(void) {
	lastDevice = NULL;
}

HRESULT IDirect3DDevice9Proxy::QueryInterface(REFIID riid, void** ppvObj) {
	*ppvObj = NULL;
	HRESULT hRes = origIDirect3DDevice9->QueryInterface(riid, ppvObj);
	if (hRes == NOERROR)
		*ppvObj = this;
	return hRes;
}

ULONG IDirect3DDevice9Proxy::AddRef(void) {
	return(origIDirect3DDevice9->AddRef());
}

ULONG IDirect3DDevice9Proxy::Release(void) {
	ULONG count = origIDirect3DDevice9->Release();
	if (count == 0) {
		delete(this);
	}
	return (count);
}

HRESULT IDirect3DDevice9Proxy::TestCooperativeLevel(void) {
	return(origIDirect3DDevice9->TestCooperativeLevel());
}

UINT IDirect3DDevice9Proxy::GetAvailableTextureMem(void) {
	return(origIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT IDirect3DDevice9Proxy::EvictManagedResources(void) {
	return(origIDirect3DDevice9->EvictManagedResources());
}

HRESULT IDirect3DDevice9Proxy::GetDirect3D(IDirect3D9** ppD3D9) {
	return(origIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT IDirect3DDevice9Proxy::GetDeviceCaps(D3DCAPS9* pCaps) {
	return(origIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT IDirect3DDevice9Proxy::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) {
	return(origIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT IDirect3DDevice9Proxy::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) {
	return(origIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT IDirect3DDevice9Proxy::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) {
	return(origIDirect3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
}

void IDirect3DDevice9Proxy::SetCursorPosition(int X, int Y, DWORD Flags) {
	return(origIDirect3DDevice9->SetCursorPosition(X, Y, Flags));
}

BOOL IDirect3DDevice9Proxy::ShowCursor(BOOL bShow) {
	return(origIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT IDirect3DDevice9Proxy::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) {
	return(origIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
}

HRESULT IDirect3DDevice9Proxy::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) {
	return(origIDirect3DDevice9->GetSwapChain(iSwapChain, pSwapChain));
}

UINT IDirect3DDevice9Proxy::GetNumberOfSwapChains(void) {
	return(origIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT IDirect3DDevice9Proxy::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	// 1. Libération des ressources dépendantes du device (avant reset)
	if (g_vegetationEffect) { g_vegetationEffect->OnLostDevice(); }

	// 2. On libère toutes les textures végétation connues
	for (auto& pair : g_vegetationTexturesByHash) {
		if (pair.second) pair.second->Release();
	}
	g_vegetationTexturesByHash.clear();

	// 3. Callback utilisateur avant reset (si besoin)
	if (callbacks[PRERESET])
		((D3D9DevicePreResetFunc)callbacks[PRERESET])();

	// 4. Reset du vrai device
	HRESULT res = origIDirect3DDevice9->Reset(pPresentationParameters);

	// 5. Réinitialisation des effets dépendants
	if (SUCCEEDED(res))
	{
		if (g_vegetationEffect) { g_vegetationEffect->OnResetDevice(); }
	}

	// 6. Callback utilisateur après reset (si besoin)
	if (callbacks[POSTRESET])
		((D3D9DevicePostResetFunc)callbacks[POSTRESET])(this, res);

	return res;
}




HRESULT IDirect3DDevice9Proxy::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	
	// Appel à la fonction Present originale
	return origIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}
	



HRESULT IDirect3DDevice9Proxy::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
	return(origIDirect3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
}

HRESULT IDirect3DDevice9Proxy::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) {
	return(origIDirect3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus));
}

HRESULT IDirect3DDevice9Proxy::SetDialogBoxMode(BOOL bEnableDialogs) {
	return(origIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void IDirect3DDevice9Proxy::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp) {
	return(origIDirect3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp));
}

void IDirect3DDevice9Proxy::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) {
	return(origIDirect3DDevice9->GetGammaRamp(iSwapChain, pRamp));
}

HRESULT IDirect3DDevice9Proxy::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return origIDirect3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT IDirect3DDevice9Proxy::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) {
	return(origIDirect3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint));
}

HRESULT IDirect3DDevice9Proxy::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) {
	return(origIDirect3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture));
}

HRESULT IDirect3DDevice9Proxy::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) {
	return(origIDirect3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface));
}

HRESULT IDirect3DDevice9Proxy::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) {
	return(origIDirect3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface));
}

HRESULT IDirect3DDevice9Proxy::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) {
	return(origIDirect3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter));
}

HRESULT IDirect3DDevice9Proxy::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color) {
	return(origIDirect3DDevice9->ColorFill(pSurface, pRect, color));
}

HRESULT IDirect3DDevice9Proxy::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) {
	return(origIDirect3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle));
}



HRESULT IDirect3DDevice9Proxy::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	// On laisse le jeu continuer son opération normalement
	return origIDirect3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT IDirect3DDevice9Proxy::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) {
	return(origIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT IDirect3DDevice9Proxy::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) {
	return(origIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT IDirect3DDevice9Proxy::BeginScene()
{
	if (g_triggerNextFrameCapture)
	{
		g_triggerNextFrameCapture = true;
		g_captureNextFrame = false;
		OutputDebugStringA("[LOG] Capture déclenchée automatiquement\n");
	}

	if (GetAsyncKeyState(VK_F12) & 1)  // déclenchement unique
	{
		g_captureNextFrame = true;
		OutputDebugStringA("[LOG] Capture prochaine frame (F12)\n");
	}

	// Ajoute le délimiteur "frame start" au début de chaque frame capturée
	if (g_captureNextFrame && endSceneCallCount == 0)
	{
		g_frameLog << "---frame " << std::setw(3) << std::setfill('0') << g_frameIndex << " start---\n";
	}

	return origIDirect3DDevice9->BeginScene();
}


//NOTE IMPORTANTE///
// ce jeu de merde appelle EndScene deux fois par image. Ca bouffe les effets si vous ne le forcez pas à ne traiter que le deuxieme appel.
// Il faut donc compter les appels et ne faire le traitement qu'au second appel.




HRESULT IDirect3DDevice9Proxy::EndScene(void)
{
	// Ce jeu appelle EndScene deux fois par image.
	// [REMPLACÉ] On ne fait plus rien, juste forward l'appel natif
	return origIDirect3DDevice9->EndScene();
}



HRESULT IDirect3DDevice9Proxy::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil){
	return(origIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT IDirect3DDevice9Proxy::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->MultiplyTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::SetViewport(CONST D3DVIEWPORT9* pViewport){
	return(origIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT IDirect3DDevice9Proxy::GetViewport(D3DVIEWPORT9* pViewport){
	return(origIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT IDirect3DDevice9Proxy::SetMaterial(CONST D3DMATERIAL9* pMaterial){
	return(origIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT IDirect3DDevice9Proxy::GetMaterial(D3DMATERIAL9* pMaterial){
	return(origIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT IDirect3DDevice9Proxy::SetLight(DWORD Index,CONST D3DLIGHT9* pLight){
	return(origIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT IDirect3DDevice9Proxy::GetLight(DWORD Index,D3DLIGHT9* pLight){
	return(origIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT IDirect3DDevice9Proxy::LightEnable(DWORD Index,BOOL Enable){
	return(origIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT IDirect3DDevice9Proxy::GetLightEnable(DWORD Index,BOOL* pEnable){
	return(origIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT IDirect3DDevice9Proxy::SetClipPlane(DWORD Index,CONST float* pPlane){
	return(origIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT IDirect3DDevice9Proxy::GetClipPlane(DWORD Index,float* pPlane){
	return(origIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT IDirect3DDevice9Proxy::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value){
	return(origIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT IDirect3DDevice9Proxy::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue){
	return(origIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT IDirect3DDevice9Proxy::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB){
	return(origIDirect3DDevice9->CreateStateBlock(Type,ppSB));
}

HRESULT IDirect3DDevice9Proxy::BeginStateBlock(void){
	return(origIDirect3DDevice9->BeginStateBlock());
}

HRESULT IDirect3DDevice9Proxy::EndStateBlock(IDirect3DStateBlock9** ppSB){
	return(origIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT IDirect3DDevice9Proxy::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus){
	return(origIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT IDirect3DDevice9Proxy::GetClipStatus(D3DCLIPSTATUS9* pClipStatus){
	return(origIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT IDirect3DDevice9Proxy::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture){
	return(origIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT IDirect3DDevice9Proxy::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	uint32_t texHash = 0;
	UINT width = 0, height = 0;
	D3DFORMAT format = D3DFMT_UNKNOWN;

	if (pTexture && pTexture->GetType() == D3DRTYPE_TEXTURE)
	{
		IDirect3DTexture9* tex = static_cast<IDirect3DTexture9*>(pTexture);

		// Récupère le hash s'il existe, sinon le calcule et le stocke
		auto it = g_textureHashes.find(pTexture);
		if (it != g_textureHashes.end()) {
			texHash = it->second;
		}
		else {
			D3DSURFACE_DESC desc;
			if (SUCCEEDED(tex->GetLevelDesc(0, &desc))) {
				width = desc.Width;
				height = desc.Height;
				format = desc.Format;

				IDirect3DSurface9* surface = nullptr;
				if (SUCCEEDED(tex->GetSurfaceLevel(0, &surface))) {
					D3DLOCKED_RECT locked;
					if (SUCCEEDED(surface->LockRect(&locked, nullptr, D3DLOCK_READONLY))) {
						size_t sampleSize = std::min<size_t>(locked.Pitch * 16, locked.Pitch * desc.Height);
						texHash = HashTextureMemory(locked.pBits, sampleSize);
						g_textureHashes[pTexture] = texHash; // Stocke le hash
						surface->UnlockRect();
					}
					surface->Release();
				}
			}
		}

		// Slot 0 = principale texture de dessin
		if (Stage == 0) {
			g_lastTextureHash = texHash;

			// Si c'est une texture végétation, on la stocke dans la map globale si absente
			if (knownVegetationHashes.find(texHash) != knownVegetationHashes.end()) {
				if (g_vegetationTexturesByHash.find(texHash) == g_vegetationTexturesByHash.end()) {
					if (tex) {
						tex->AddRef();
						g_vegetationTexturesByHash[texHash] = tex;
					}
				}
			}
		}
	}

	// Appel direct à la fonction d'origine
	return origIDirect3DDevice9->SetTexture(Stage, pTexture);
}









HRESULT IDirect3DDevice9Proxy::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
	return(origIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT IDirect3DDevice9Proxy::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	return(origIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT IDirect3DDevice9Proxy::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
	return(origIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT IDirect3DDevice9Proxy::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	return(origIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT IDirect3DDevice9Proxy::ValidateDevice(DWORD* pNumPasses)
{
	return(origIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT IDirect3DDevice9Proxy::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
	return(origIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT IDirect3DDevice9Proxy::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
	return(origIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT IDirect3DDevice9Proxy::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return(origIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT IDirect3DDevice9Proxy::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return(origIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT IDirect3DDevice9Proxy::SetScissorRect(CONST RECT* pRect)
{
	return(origIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT IDirect3DDevice9Proxy::GetScissorRect( RECT* pRect)
{
	return(origIDirect3DDevice9->GetScissorRect( pRect));
}

HRESULT IDirect3DDevice9Proxy::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return(origIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    IDirect3DDevice9Proxy::GetSoftwareVertexProcessing(void)
{
	return(origIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT IDirect3DDevice9Proxy::SetNPatchMode(float nSegments)
{
	return(origIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   IDirect3DDevice9Proxy::GetNPatchMode(void)
{
	return(origIDirect3DDevice9->GetNPatchMode());
}

HRESULT IDirect3DDevice9Proxy::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	IDirect3DBaseTexture9* tex = nullptr;
	origIDirect3DDevice9->GetTexture(0, &tex);

	char buf[512];
	sprintf_s(buf, "[DrawPrimitive] Type=%d, StartVertex=%u, Count=%u, Tex0=%p\n",
		PrimitiveType, StartVertex, PrimitiveCount, tex);
	OutputDebugStringA(buf);

	if (tex) tex->Release();
	return origIDirect3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}


// =====================================================================================
// [VEGETATION HOOK] - DAoC D3D9 Fixed Pipeline Override (végétation animée)
//
// [FR]
// Ce hook intercepte les appels à DrawIndexedPrimitive pour les sprites de végétation
// dans Dark Age of Camelot (client Direct3D9). Il est basé sur une analyse préalable
// du pipeline graphique du jeu, qui révèle que la végétation est rendue via :
//
//   - Des quads alpha-blendés (billboards), avec de petits index buffers,
//   - Des ressources identifiables par hash de texture (prévisible),
//   - Un pipeline fixe (Fixed Function Pipeline), sans vertex/pixel shaders,
//   - Des états parfois incorrects : textures non bindées, COLOROP désactivé.
//
// Le hook applique un effet (pixel shader) pour animer les sprites (e.g., vent),
// tout en conservant le modèle d'origine.
//
// Points clés :
//  - Le rendu végétation est identifié par g_lastTextureHash, ( voir commentaire dans Texturehash.cpp)
//  - Une texture est rebindée si nécessaire,
//  - Les TextureStageStates sont forcés (sinon les sprites apparaissent noirs),
//  - Les matrices World/View/Projection sont passées à l'effet,
//  - Le contexte Direct3D est sauvegardé et restauré via IDirect3DStateBlock9,
//  - Le fallback vers DrawIndexedPrimitive normal est conservé si erreur.
//
// [EN]
// This hook overrides vegetation sprite rendering in Dark Age of Camelot (D3D9 client).
// Based on reverse-engineering the game's render path, we determined that vegetation is:
//
//   - Rendered as alpha-blended quads (billboards) using small index buffers,
//   - Identified by predictable texture hashes,
//   - Processed through the fixed-function pipeline (no shaders),
//   - Occasionally broken: unbound textures, COLOROP = DISABLE, etc.
//
// This hook injects a pixel shader-based animation effect (e.g., wind motion),
// while keeping the original geometry untouched.
//
// Key steps:
//  - Detect vegetation drawcall via g_lastTextureHash,(see comment in Texturehash.cpp)
//  - Bind the correct texture if needed,
//  - Force texture stage states to prevent blank output,
//  - Pass WVP matrices and animation time to the shader,
//  - Capture & restore device state using IDirect3DStateBlock9,
//  - Fallback to original DIP if effect or hash isn't valid.
//
// This is part of a larger effort to modernize DAoC’s rendering path incrementally.
// =====================================================================================





HRESULT IDirect3DDevice9Proxy::DrawIndexedPrimitive(
	D3DPRIMITIVETYPE PrimitiveType,
	INT BaseVertexIndex,
	UINT MinVertexIndex,
	UINT NumVertices,
	UINT StartIndex,
	UINT PrimCount)
{
	drawCallCount++;

	// See texturehash.cpp for details on hash-based vegetation detection.
// (This is a temporary dev-stage solution; see notes there for future plan.)

	bool isVegetation = (g_detectedVegetationHashes.find(g_lastTextureHash) != g_detectedVegetationHashes.end());

	if (isVegetation && g_vegetationEffect)
	{
		HRESULT result = D3D_OK;
		IDirect3DStateBlock9* pStateBlock = nullptr;



		if (SUCCEEDED(origIDirect3DDevice9->CreateStateBlock(D3DSBT_ALL, &pStateBlock)))
		{
			pStateBlock->Capture();

			// 1. Cherche la bonne texture végétation par hash
			// 1. Look up the corresponding vegetation texture using its hashed ID
			auto it = g_vegetationTexturesByHash.find(g_lastTextureHash);
			IDirect3DTexture9* wantedTex = (it != g_vegetationTexturesByHash.end()) ? it->second : nullptr;

			// 2. Récupère la texture courante bindée sur slot 0
			// 2. Retrieve the currently bound texture from texture stage 0
			IDirect3DBaseTexture9* currentTex = nullptr;
			origIDirect3DDevice9->GetTexture(0, &currentTex);

			// 3. Si c’est pas la bonne, on la bind explicitement
			// 3. If the bound texture differs from the expected one, override it manually
			if (wantedTex && currentTex != static_cast<IDirect3DBaseTexture9*>(wantedTex))

				origIDirect3DDevice9->SetTexture(0, wantedTex);

			// 4. Passe la texture (celle qui est bindée, donc wantedTex ou currentTex) à l’effet
			// 4. Set the bound texture (either wantedTex or currentTex) as input for the shader
			IDirect3DBaseTexture9* shaderTex = wantedTex ? wantedTex : currentTex;
		

			if (shaderTex)
				g_vegetationEffect->SetTexture(g_baseTextureHandle, shaderTex);

			// 5. Animation
			// 5. Update animation time parameter (based on system clock)
			float timeSeconds = static_cast<float>(GetTickCount64() % 100000) / 1000.0f;
			g_vegetationEffect->SetFloat(g_timeHandle, timeSeconds);

			// 6. Matrices
			// 6. Compute and pass the World-View-Projection matrix to the shader
			D3DXMATRIX world, view, proj, wvp;
			origIDirect3DDevice9->GetTransform(D3DTS_WORLD, &world);
			origIDirect3DDevice9->GetTransform(D3DTS_VIEW, &view);
			origIDirect3DDevice9->GetTransform(D3DTS_PROJECTION, &proj);
			wvp = world * view * proj;
			g_vegetationEffect->SetMatrix("WorldViewProjection", &wvp);

			// --- Forcer les TextureStageStates pour le slot 0 (Fixed Pipeline) ---
			// --- Ensure fixed-function texture stage states are correctly set for modulation ---)
			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			origIDirect3DDevice9->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);


			g_vegetationEffect->Begin(nullptr, 0);
			g_vegetationEffect->BeginPass(0);

			result = origIDirect3DDevice9->DrawIndexedPrimitive(
				PrimitiveType, BaseVertexIndex, MinVertexIndex,
				NumVertices, StartIndex, PrimCount);

			g_vegetationEffect->EndPass();
			g_vegetationEffect->End();

			pStateBlock->Apply();
			pStateBlock->Release();

			if (currentTex) currentTex->Release();

			return result;
		}
		// Fallback si erreur
		// Fallback: perform draw call without effect if state block creation fails
		return origIDirect3DDevice9->DrawIndexedPrimitive(
			PrimitiveType, BaseVertexIndex, MinVertexIndex,
			NumVertices, StartIndex, PrimCount);
	}

	// Cas normal
	// Default case: forward the call without any vegetation-specific processing
	return origIDirect3DDevice9->DrawIndexedPrimitive(
		PrimitiveType, BaseVertexIndex, MinVertexIndex,
		NumVertices, StartIndex, PrimCount);
}







HRESULT IDirect3DDevice9Proxy::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	return(origIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT IDirect3DDevice9Proxy::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	return(origIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT IDirect3DDevice9Proxy::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	return(origIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	return(origIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
}

HRESULT IDirect3DDevice9Proxy::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	if (g_bEnableDrawCallLogging)
	{
		logVertexDecl << "SetVertexDeclaration(Decl=" << pDecl << ")\n";
	}
	return(origIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT IDirect3DDevice9Proxy::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return(origIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT IDirect3DDevice9Proxy::SetFVF(DWORD FVF)
{
	if (g_bEnableDrawCallLogging)
	{
		logFVF << "SetFVF(FVF=0x" << std::hex << FVF << std::dec << ")\n";
	}

	return(origIDirect3DDevice9->SetFVF(FVF));
}

HRESULT IDirect3DDevice9Proxy::GetFVF(DWORD* pFVF)
{
	return(origIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	return(origIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}
HRESULT IDirect3DDevice9Proxy::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	if (g_bEnableDrawCallLogging)
	{
		logVertexShader << "SetVertexShader(pShader=" << pShader << ")\n";
	}


	return origIDirect3DDevice9->SetVertexShader(pShader);
}

HRESULT IDirect3DDevice9Proxy::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return(origIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	if (g_bEnableDrawCallLogging)
	{
		logStream << "SetStreamSource(Stream=" << StreamNumber
			<< ", VB=" << pStreamData
			<< ", Offset=" << OffsetInBytes
			<< ", Stride=" << Stride << ")\n";
	}


	if (StreamNumber == 0)
	{
		IDirect3DBaseTexture9* tex = nullptr;
		origIDirect3DDevice9->GetTexture(0, &tex);

		char buf[512];
		sprintf_s(buf, "[SetStreamSource] Stream=%u, Offset=%u, Stride=%u, VBuffer=%p, Tex0=%p\n",
			StreamNumber, OffsetInBytes, Stride, pStreamData, tex);
		OutputDebugStringA(buf);

		if (tex) tex->Release();
	}

	return origIDirect3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}


HRESULT IDirect3DDevice9Proxy::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
	return(origIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT IDirect3DDevice9Proxy::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
	return(origIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT IDirect3DDevice9Proxy::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
	return(origIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT IDirect3DDevice9Proxy::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	if (g_bEnableDrawCallLogging)
	{
		logVertexBuffer << "SetIndices(IndexBuffer=" << pIndexData << ")\n";
	}

	return(origIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT IDirect3DDevice9Proxy::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return(origIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT IDirect3DDevice9Proxy::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	return(origIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	if (g_bEnableDrawCallLogging)
	{
		logPixelShader << "SetPixelShader(pShader=" << pShader << ")\n";
	}
	if (g_captureNextFrame)
	g_frameLog << "SetPixelShader(" << pShader << ")\n";

	return origIDirect3DDevice9->SetPixelShader(pShader);
}


HRESULT IDirect3DDevice9Proxy::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return(origIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return(origIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT IDirect3DDevice9Proxy::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return(origIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT IDirect3DDevice9Proxy::DeletePatch(UINT Handle)
{
	return(origIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT IDirect3DDevice9Proxy::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	return(origIDirect3DDevice9->CreateQuery(Type,ppQuery));
}
HRESULT IDirect3DDevice9Proxy::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return origIDirect3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}