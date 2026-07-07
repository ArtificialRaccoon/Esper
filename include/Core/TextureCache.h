#pragma once
#include <unordered_map>
#include <string>
#include <allegro.h>

class TextureCache
{
	public:
		static TextureCache &Instance()
		{
			static TextureCache instance;
			return instance;
		}

		BITMAP *Get(const std::string &name);
		void Clear();

	private:
		TextureCache() = default;
		~TextureCache() { Clear(); }

	private:
		std::unordered_map<std::string, BITMAP *> cache;
};
