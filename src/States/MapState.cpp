#include "States/MapState.h"
#include <algorithm>

void MapState::InitState()
{
	BaseState::InitState();
	BUFFER = create_bitmap(VSCREEN_W, VSCREEN_H);
	PALETTE pal;
	tileset = load_bitmap(".\\TILESETS\\SHEET.bmp", pal);

	for (int i = 0; i < 256; i++)
	{
		CommonGUI::Instance().GetPalette()[i] = pal[i];
	}
	set_palette(CommonGUI::Instance().GetPalette());

	player.Load(".\\CHARS\\CHAR.bmp");

	playerMoving = false;
	currentMoveDir = Direction::DOWN;

	MapTransition(
		GameDatabase::Instance().GetStartingMapName(),
		GameDatabase::Instance().GetStartingTileX(),
		GameDatabase::Instance().GetStartingTileY()
	);
}

void MapState::MapTransition(const std::string &mapName, int targetTileX, int targetTileY)
{
	tileMap.Load(".\\MAPS\\" + mapName + ".bin");

	mapWidthPx = tileMap.GetLayers().empty() ? 0 : tileMap.GetLayers()[0].width * TILE_SIZE;
	mapHeightPx = tileMap.GetLayers().empty() ? 0 : tileMap.GetLayers()[0].height * TILE_SIZE;

	playerMapX = targetTileX * TILE_SIZE;
	playerMapY = targetTileY * TILE_SIZE;

	scrollX = std::clamp(playerMapX - (SCREEN_WIDTH - CHARACTER_SPRITE_WIDTH) / 2, 0, mapWidthPx - SCREEN_WIDTH);
	scrollY = std::clamp(playerMapY - (SCREEN_HEIGHT - CHARACTER_SPRITE_HEIGHT) / 2, 0, mapHeightPx - SCREEN_HEIGHT);

	prevScrollTileX = -1;
	prevScrollTileY = -1;
	tilemapChanged = true;
}

void MapState::AcquireInput(GameProcessor *game)
{
	int dx = 0;
	int dy = 0;
	int speed = WALK_SPEED;

	interactPressed = false;
	playerMoving = false;

	if (InputManager::Instance().IsKeyDown(KEY_UP))
	{
		dy = -speed;
		currentMoveDir = Direction::UP;
	}
	else if (InputManager::Instance().IsKeyDown(KEY_DOWN))
	{
		dy = speed;
		currentMoveDir = Direction::DOWN;
	}

	if (InputManager::Instance().IsKeyDown(KEY_LEFT))
	{
		dx = -speed;
		currentMoveDir = Direction::LEFT;
	}
	else if (InputManager::Instance().IsKeyDown(KEY_RIGHT))
	{
		dx = speed;
		currentMoveDir = Direction::RIGHT;
	}

	if (dx != 0)
	{
		int newX = std::clamp(playerMapX + dx, 0, mapWidthPx - CHARACTER_SPRITE_WIDTH);
		if (!tileMap.CheckCollision(
			newX + CHARACTER_HITBOX_X_OFFSET, 
			playerMapY + CHARACTER_HITBOX_Y_OFFSET, 
			CHARACTER_HITBOX_WIDTH, 
			CHARACTER_HITBOX_HEIGHT))
		{
			playerMapX = newX;
			playerMoving = true;
		}
	}

	if (dy != 0)
	{
		int newY = std::clamp(playerMapY + dy, 0, mapHeightPx - CHARACTER_SPRITE_HEIGHT);
		if (!tileMap.CheckCollision(
			playerMapX + CHARACTER_HITBOX_X_OFFSET, 
			newY + CHARACTER_HITBOX_Y_OFFSET, 
			CHARACTER_HITBOX_WIDTH, 
			CHARACTER_HITBOX_HEIGHT))
		{
			playerMapY = newY;
			playerMoving = true;
		}
	}

	if (InputManager::Instance().IsKeyPressed(KEY_ESC))
		interactPressed = true;
}

void MapState::ProcessInput(GameProcessor *game)
{
	int playerStartTileX = (playerMapX + CHARACTER_HITBOX_X_OFFSET) / TILE_SIZE;
	int playerEndTileX = (playerMapX + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1) / TILE_SIZE;
	int playerStartTileY = (playerMapY + CHARACTER_HITBOX_Y_OFFSET) / TILE_SIZE;
	int playerEndTileY = (playerMapY + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1) / TILE_SIZE;

	for (const auto &exit : tileMap.GetExits())
	{
		if (playerStartTileX <= exit.endTileX && playerEndTileX >= exit.startTileX &&
			playerStartTileY <= exit.endTileY && playerEndTileY >= exit.startTileY)
		{
			MapTransition(exit.targetMapId, exit.targetX, exit.targetY);
			break;
		}
	}

	scrollX = std::clamp(playerMapX - (SCREEN_WIDTH - CHARACTER_SPRITE_WIDTH) / 2, 0, mapWidthPx - SCREEN_WIDTH);
	scrollY = std::clamp(playerMapY - (SCREEN_HEIGHT - CHARACTER_SPRITE_HEIGHT) / 2, 0, mapHeightPx - SCREEN_HEIGHT);

	tilemapChanged |= tileMap.Update();
	player.Update(playerMoving, currentMoveDir);

	if (interactPressed)
		game->Quit();
}

void MapState::FrameRender(GameProcessor *game)
{
	int currentScrollTileX = scrollX / TILE_SIZE;
	int currentScrollTileY = scrollY / TILE_SIZE;
	game->SetScrollOffset(scrollX % TILE_SIZE, scrollY % TILE_SIZE);

	if (currentScrollTileX != prevScrollTileX || currentScrollTileY != prevScrollTileY || tilemapChanged)
	{
		tileMap.Draw(BUFFER, tileset, currentScrollTileX, currentScrollTileY);
		prevScrollTileX = currentScrollTileX;
		prevScrollTileY = currentScrollTileY;
		tilemapChanged = false;
	}

	blit(BUFFER, game->GetBackBuffer(), 0, 0, 0, 0, VSCREEN_W, VSCREEN_H);
	player.Draw(game->GetBackBuffer(), playerMapX - currentScrollTileX * TILE_SIZE, playerMapY - currentScrollTileY * TILE_SIZE);
	tileMap.DrawUpper(game->GetBackBuffer(), tileset, currentScrollTileX, currentScrollTileY);
}

void MapState::UnloadResources()
{
	if (tileset)
	{
		destroy_bitmap(tileset);
		tileset = nullptr;
	}

	if (BUFFER)
	{
		destroy_bitmap(BUFFER);
		BUFFER = nullptr;
	}
}