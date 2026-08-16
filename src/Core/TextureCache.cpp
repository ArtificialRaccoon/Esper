#include "Core/TextureCache.h"
#include "Core/AssetPaths.h"

BITMAP *TextureCache::Get(const std::string &name)
{
	if (name.empty())
		return nullptr;

	auto it = cache.find(name);
	if (it != cache.end())
		return it->second;

	BITMAP *bmp = load_bitmap((std::string(AssetPaths::DIR_TILESETS) + name + ".bmp").c_str(), nullptr);
	if (!bmp)
		bmp = load_bitmap((std::string(AssetPaths::DIR_CHARS) + name + ".bmp").c_str(), nullptr);

	cache[name] = bmp;
	return bmp;
}

void TextureCache::Clear()
{
	for (auto &pair : cache)
	{
		if (pair.second)
			destroy_bitmap(pair.second);
	}
	cache.clear();
}
