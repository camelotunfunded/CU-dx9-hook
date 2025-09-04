#include "TextureHash.h"

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
