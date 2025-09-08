#include "TextureHash.h"

// [FR]
// La détection de la végétation repose actuellement sur un hash de texture (g_lastTextureHash),
// ce qui fonctionne de manière fiable tant que les textures ne changent pas.
// Cependant, cette approche est temporaire : à terme, il faudra intercepter les appels
// de chargement ou de binding de texture pour récupérer dynamiquement leur nom en mémoire.
//
// Cela permettra d’identifier les textures de végétation par leur nom réel (ex: "tree_", "grass_")
// plutôt que par des valeurs hashées spécifiques au build courant, ce qui est plus robuste
// et maintenable dans la durée.

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
