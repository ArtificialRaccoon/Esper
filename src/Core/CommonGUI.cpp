#include <algorithm>
#include "Core/CommonGUI.h"
#include "Core/GameDefines.h"
#include "Core/AssetPaths.h"

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

	guiSheet = load_bitmap(AssetPaths::GUI_SHEET, tmp);

	//Initialize master palette to all black for now
	for (int i = 0; i < 256; i++)
	{
		palette[i].r = palette[i].g = palette[i].b = 0;
	}

	systemFonts.push_back(load_font(AssetPaths::FONT_BITSCRIP, tmp, NULL));
	systemFonts.push_back(load_font(AssetPaths::FONT_ARCANA, tmp, NULL));
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

void CommonGUI::DrawNineSliceBox(BITMAP *dest, int x, int y, int width, int height)
{
	if (!dest || !guiSheet || width <= 0 || height <= 0)
		return;

	masked_blit(guiSheet, dest, 0, 0, x, y, TILE_SIZE, TILE_SIZE);
	masked_blit(guiSheet, dest, TILE_SIZE * 2, 0, x + width - TILE_SIZE, y, TILE_SIZE, TILE_SIZE);
	masked_blit(guiSheet, dest, 0, TILE_SIZE * 2, x, y + height - TILE_SIZE, TILE_SIZE, TILE_SIZE);
	masked_blit(guiSheet, dest, TILE_SIZE * 2, TILE_SIZE * 2, x + width - TILE_SIZE, y + height - TILE_SIZE, TILE_SIZE, TILE_SIZE);

	for (int px = x + TILE_SIZE; px < x + width - TILE_SIZE; px += TILE_SIZE)
	{
		int drawWidth = std::min(TILE_SIZE, (x + width - TILE_SIZE) - px);
		masked_blit(guiSheet, dest, TILE_SIZE, 0, px, y, drawWidth, TILE_SIZE);
		masked_blit(guiSheet, dest, TILE_SIZE, TILE_SIZE * 2, px, y + height - TILE_SIZE, drawWidth, TILE_SIZE);
	}

	for (int py = y + TILE_SIZE; py < y + height - TILE_SIZE; py += TILE_SIZE)
	{
		int drawHeight = std::min(TILE_SIZE, (y + height - TILE_SIZE) - py);
		masked_blit(guiSheet, dest, 0, TILE_SIZE, x, py, TILE_SIZE, drawHeight);
		masked_blit(guiSheet, dest, TILE_SIZE * 2, TILE_SIZE, x + width - TILE_SIZE, py, TILE_SIZE, drawHeight);
	}

	for (int py = y + TILE_SIZE; py < y + height - TILE_SIZE; py += TILE_SIZE)
	{
		int drawHeight = std::min(TILE_SIZE, (y + height - TILE_SIZE) - py);
		for (int px = x + TILE_SIZE; px < x + width - TILE_SIZE; px += TILE_SIZE)
		{
			int drawWidth = std::min(TILE_SIZE, (x + width - TILE_SIZE) - px);
			masked_blit(guiSheet, dest, TILE_SIZE, TILE_SIZE, px, py, drawWidth, drawHeight);
		}
	}
}