#pragma once

#include <set>
#include <unordered_map>
#include <fstream>
#include <d3d9.h>
#include <string>
#include <cstdint>
#include <sstream> // Inclure pour std::stringstream

// Interrupteur pour l'export JPG
extern bool g_bEnableJpgExport;
// Interrupteur pour le log des draw calls
extern bool g_bEnableDrawCallLogging;


extern std::set<IDirect3DBaseTexture9*> g_loggedTextures;

extern bool g_captureNextFrame;

extern int g_frameIndex;

void InitTextureLog();
void ExportTextureAsJPG(IDirect3DTexture9* tex, const std::string& filename);

;
extern std::stringstream logDrawIndexed;
extern std::stringstream logFVF;
extern std::stringstream logStream;
extern std::stringstream logVertexDecl;
extern std::stringstream logVertexBuffer;
extern std::stringstream logVertexShader;
extern std::stringstream logPixelShader;
extern std::stringstream g_frameLog;
extern std::stringstream g_textureLog;

// Fonction pour vider tous les tampons dans les fichiers
void FlushLogs();

// --- Nouvelles variables globales ---
extern std::set<uint32_t> g_detectedVegetationHashes;
extern std::unordered_map<IDirect3DBaseTexture9*, uint32_t> g_textureHashes;
extern uint32_t g_lastTextureHash;
