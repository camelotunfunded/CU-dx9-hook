#include "TextureLogger.h"
#include <Windows.h>
#include <filesystem>
#include <unordered_set>

// D�finition de l'interrupteur, d�sactiv� par d�faut pour de meilleures performances.
bool g_bEnableJpgExport = false;
bool g_bEnableDrawCallLogging = false;

bool g_triggerNextFrameCapture = false;
// D�clarations des fichiers de log suppl�mentaires
std::stringstream logDrawIndexed;
std::stringstream logFVF;
std::stringstream logStream;
std::stringstream logVertexDecl;
std::stringstream logVertexBuffer;
std::stringstream logVertexShader;
std::stringstream logPixelShader;
std::stringstream g_frameLog;
std::stringstream g_textureLog;
int g_frameIndex = 0;

void InitTextureLog()
{
    // �crit le message de d�marrage dans le tampon en m�moire.
    // Il sera sauvegard� dans le fichier lors du prochain appel � FlushLogs().
    g_textureLog << "=== Logging started ===\n";
}

void FlushLogs()
{
    // Cr�e le r�pertoire principal si n�cessaire
    CreateDirectoryA("C:\\D3D9Proxy", NULL);

    // Fonction pour �crire un tampon dans un fichier
    auto writeBufferToFile = [](const std::string& path, std::stringstream& buffer) {
        // N'�crit rien si le tampon est vide
        if (buffer.str().empty()) return;

        std::ofstream file(path, std::ios::app); // Ouvre en mode ajout
        if (file.is_open()) {
            file << buffer.rdbuf(); // Vide le contenu du stringstream dans le fichier
        }
        // buffer.str(""); // Vide le stringstream pour lib�rer la RAM.
        // buffer.clear(); // Efface les indicateurs d'erreur du stream.
        };

    // Vider chaque tampon dans son fichier respectif
    writeBufferToFile("C:\\D3D9Proxy\\log_drawindexed.txt", logDrawIndexed);
    writeBufferToFile("C:\\D3D9Proxy\\log_fvf.txt", logFVF);
    writeBufferToFile("C:\\D3D9Proxy\\log_stream.txt", logStream);
    writeBufferToFile("C:\\D3D9Proxy\\log_vertexdecl.txt", logVertexDecl);
    writeBufferToFile("C:\\D3D9Proxy\\log_vertexbuffer.txt", logVertexBuffer);
    writeBufferToFile("C:\\D3D9Proxy\\log_vertexshader.txt", logVertexShader);
    writeBufferToFile("C:\\D3D9Proxy\\log_pixelshader.txt", logPixelShader);
    writeBufferToFile("C:\\D3D9Proxy\\frame_log.txt", g_frameLog);
    writeBufferToFile("C:\\D3D9Proxy\\textures.log", g_textureLog);
}

bool g_captureNextFrame = false;

std::unordered_set<uint32_t> knownVegetationHashes = {
    0x101ECAB5 //atlas alb
};

std::set<IDirect3DBaseTexture9*> g_loggedTextures;







#include <d3dx9.h>

void ExportTextureAsJPG(IDirect3DTexture9* tex, const std::string& filename)
{
    if (!tex) return;

    std::string fullPath = "C:\\D3D9Proxy\\jpg\\" + filename + ".jpg";

    // Sauvegarde la texture au format JPG
    D3DXSaveTextureToFileA(
        fullPath.c_str(),
        D3DXIFF_JPG,
        tex,
        nullptr
    );
}
// --- Impl�mentation des nouvelles variables globales ---
std::set<uint32_t> g_detectedVegetationHashes;
std::unordered_map<IDirect3DBaseTexture9*, uint32_t> g_textureHashes;
uint32_t g_lastTextureHash = 0;
