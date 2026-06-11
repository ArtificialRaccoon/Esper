#pragma once
#include <vector>
#include <stdio.h>
#include "allegro.h"

class CommonGUI
{
	public:
		~CommonGUI();
		static CommonGUI &Instance()
		{
			static CommonGUI mCommonGUI;
			return mCommonGUI;
		}

		BITMAP *GetBitmap() { return guiSheet; }
		PALETTE &GetPalette() { return palette; }
		FONT *GetFont(int fontIndex) { return systemFonts[fontIndex]; }

		void LoadPalette(const char *filename);
		void LoadPalette(const char *filename, int startIndex, int endIndex);

	private:
		CommonGUI();

		BITMAP *guiSheet = nullptr;
		PALETTE palette;
		std::vector<FONT *> systemFonts;
};