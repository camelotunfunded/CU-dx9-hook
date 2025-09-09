#pragma once

#include <set>
#include <unordered_map>
#include <fstream>
#include <d3d9.h>
#include <string>
#include <cstdint>
#include <sstream> // Include for std::stringstream

// Switch for JPG export
extern bool g_bEnableJpgExport;
// Switch for draw call logging
extern bool g_bEnableDrawCallLogging;
extern bool g_bEnableTextureLog;
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

// Function to flush all buffers to files
void FlushLogs();

// --- New global variables ---
extern std::set<uint32_t> g_detectedVegetationHashes;
extern std::unordered_map<IDirect3DBaseTexture9*, uint32_t> g_textureHashes;
extern uint32_t g_lastTextureHash;
