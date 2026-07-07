#include "Core/CommonGUI.h"

CommonGUI::~CommonGUI()
{
	for (auto fontObj : systemFonts)
	{
		if (fontObj)
			destroy_font(fontObj);
	}

	if (guiSheet)
	{
		destroy_bitmap(guiSheet);
		guiSheet = nullptr;
	}
}

CommonGUI::CommonGUI()
{
	PALETTE tmp;

	guiSheet = load_bitmap(".\\OTHER\\GUI.bmp", tmp);

	//Initialize master palette to all black for now
	for (int i = 0; i < 256; i++)
	{
		palette[i].r = palette[i].g = palette[i].b = 0;
	}

	systemFonts.push_back(load_font(".\\OTHER\\BitScrip.bmp", tmp, NULL));
	systemFonts.push_back(load_font(".\\OTHER\\Arcana.bmp", tmp, NULL));
	font = systemFonts.front();

	//Load palette ranges from file here...
	set_palette(palette);
}

void CommonGUI::LoadPalette(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
		return;

	for (int i = 0; i < 256; i++)
	{
		palette[i].r = fgetc(file);
		palette[i].g = fgetc(file);
		palette[i].b = fgetc(file);
	}
	fclose(file);
	set_palette(palette);
}

void CommonGUI::LoadPalette(const char *filename, int startIndex, int endIndex)
{
	if (startIndex < 0 || endIndex > 255 || startIndex > endIndex)
		return;

	FILE *file = fopen(filename, "rb");
	if (!file)
		return;

	PALETTE temp;
	for (int i = 0; i < 256; i++)
	{
		temp[i].r = fgetc(file);
		temp[i].g = fgetc(file);
		temp[i].b = fgetc(file);
	}
	fclose(file);

	for (int i = startIndex; i <= endIndex; i++) { palette[i] = temp[i]; }
	set_palette_range(palette, startIndex, endIndex, FALSE);
}