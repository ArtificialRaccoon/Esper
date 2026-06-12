#pragma once
#include <stdexcept>
#include <cstdio>
#include <algorithm>
#include "GameProcessor.h"
#include "States/BaseState.h"
#include "Core/GameDefines.h"
#include "Core/CommonGUI.h"
#include "Core/GameDatabase.h"
#include "Core/Player.h"
#include "Core/TileMap.h"
#include "Core/GlobalEnumerations.h"
#include "Utilities/InputManager.h"
#include "Utilities/AudioManager.h"

class MapState : public BaseState
{
	public:
		MapState() = default;
		~MapState() override { UnloadResources(); }

		void InitState() override;
		void Pause() override { }
		void Resume() override { set_palette(CommonGUI::Instance().GetPalette()); }
		void AcquireInput(GameProcessor *game) override;
		void ProcessInput(GameProcessor *game) override;
		void FrameRender(GameProcessor *game) override;
		void UnloadResources() override;

	private:
		TileMap tileMap;
		BITMAP *tileset = nullptr;
		Player player;
		Direction currentMoveDir = Direction::DOWN;		
		int scrollX = 0;
		int scrollY = 0;
		int playerMapX = 0;
		int playerMapY = 0;
		int mapWidthPx = 0;
		int mapHeightPx = 0;
		int prevScrollTileX = -1;
		int prevScrollTileY = -1;
		bool tilemapChanged = false;
		bool playerMoving = false;
		
	private:
		void MapTransition(const std::string &mapName, int targetTileX, int targetTileY);
};