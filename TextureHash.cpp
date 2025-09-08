#include "TextureHash.h"

// [FR]
// La d�tection de la v�g�tation repose actuellement sur un hash de texture (g_lastTextureHash),
// ce qui fonctionne de mani�re fiable tant que les textures ne changent pas.
// Cependant, cette approche est temporaire : � terme, il faudra intercepter les appels
// de chargement ou de binding de texture pour r�cup�rer dynamiquement leur nom en m�moire.
//
// Cela permettra d�identifier les textures de v�g�tation par leur nom r�el (ex: "tree_", "grass_")
// plut�t que par des valeurs hash�es sp�cifiques au build courant, ce qui est plus robuste
// et maintenable dans la dur�e.

// [EN]
// Vegetation detection is currently based on a texture hash (g_lastTextureHash),
// which works reliably as long as texture IDs remain stable.
// However, this is a temporary approach: in the future, we plan to intercept texture
// loading or binding calls to dynamically extract their names from memory.
//
// This will allow us to identify vegetation textures by their actual names (e.g., "tree_", "grass_")
// instead of hardcoded hash values, making the system more robust and maintainable long-term.




uint32_t HashTextureMemory(const void* data, size_t size)
{
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
	uint32_t hash = 2166136261u;

	for (size_t i = 0; i < size; ++i)
	{
		hash ^= bytes[i];
		hash *= 16777619u;
	}

	return hash;
}
