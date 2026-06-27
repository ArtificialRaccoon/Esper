#include "States/MapState.h"
#include "Core/GameState.h"

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

	AudioManager::Instance().PlayMusic("ff3town");
}

void MapState::MapTransition(const std::string &mapName, int targetTileX, int targetTileY)
{
	currentMapName = mapName;
	tileMap.Load(".\\MAPS\\" + mapName + ".bin");

	for (auto &pair : eventSprites)
	{
		if (pair.second)
			destroy_bitmap(pair.second);
	}
	eventSprites.clear();

	for (auto &event : tileMap.GetEvents())
	{
		event.UpdateActivePage(currentMapName);
		for (const auto &page : event.GetPages())
		{
			if (page.spriteName[0] != '\0')
			{
				std::string sName = page.spriteName;
				if (eventSprites.find(sName) == eventSprites.end())
				{
					std::string path = ".\\TILESETS\\" + sName + ".bmp";
					BITMAP* bmp = load_bitmap(path.c_str(), nullptr);
					if (bmp)
						eventSprites[sName] = bmp;
				}
			}
		}
	}

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

bool MapState::CheckEventCollision(int targetX, int targetY)
{
	int startTileX = targetX / TILE_SIZE;
	int endTileX = (targetX + CHARACTER_HITBOX_WIDTH - 1) / TILE_SIZE;
	int startTileY = targetY / TILE_SIZE;
	int endTileY = (targetY + CHARACTER_HITBOX_HEIGHT - 1) / TILE_SIZE;

	for (auto &event : tileMap.GetEvents())
	{
		event.UpdateActivePage(currentMapName);
		const EventPage* activePage = event.GetActivePage();
		if (activePage && activePage->isWalkable == 0)
		{
			int eventStartX = event.GetTileX();
			int eventEndX = event.GetEndTileX();
			int eventStartY = event.GetTileY();
			int eventEndY = event.GetEndTileY();

			if (startTileX <= eventEndX && endTileX >= eventStartX && startTileY <= eventEndY && endTileY >= eventStartY)
				return true;
		}
	}
	return false;
}

void MapState::AcquireInput(GameProcessor *game)
{
	interactPressed = false;
	playerMoving = false;

	if (InputManager::Instance().IsKeyPressed(KEY_ENTER) || InputManager::Instance().IsKeyPressed(KEY_SPACE))
		interactPressed = true;

	//My idea is to have a series of "overlays" which we will generically access.
	if (dialogBox.IsActive())
		return;

	int dx = 0;
	int dy = 0;
	int speed = WALK_SPEED;

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
		int targetX = newX + CHARACTER_HITBOX_X_OFFSET;
		int targetY = playerMapY + CHARACTER_HITBOX_Y_OFFSET;
		if (!tileMap.CheckCollision(targetX, targetY, CHARACTER_HITBOX_WIDTH, CHARACTER_HITBOX_HEIGHT) && !CheckEventCollision(targetX, targetY))
		{
			playerMapX = newX;
			playerMoving = true;
		}
	}

	if (dy != 0)
	{
		int newY = std::clamp(playerMapY + dy, 0, mapHeightPx - CHARACTER_SPRITE_HEIGHT);
		int targetX = playerMapX + CHARACTER_HITBOX_X_OFFSET;
		int targetY = newY + CHARACTER_HITBOX_Y_OFFSET;
		if (!tileMap.CheckCollision(targetX, targetY, CHARACTER_HITBOX_WIDTH, CHARACTER_HITBOX_HEIGHT) && !CheckEventCollision(targetX, targetY))
		{
			playerMapY = newY;
			playerMoving = true;
		}
	}

	if (InputManager::Instance().IsKeyPressed(KEY_ESC))
		game->Quit();
}

void MapState::ProcessInput(GameProcessor *game)
{
	if (dialogBox.IsActive())
	{
		dialogBox.Update();
		if (interactPressed)
			dialogBox.Advance();
		return;
	}

	int playerStartTileX = (playerMapX + CHARACTER_HITBOX_X_OFFSET) / TILE_SIZE;
	int playerEndTileX = (playerMapX + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1) / TILE_SIZE;
	int playerStartTileY = (playerMapY + CHARACTER_HITBOX_Y_OFFSET) / TILE_SIZE;
	int playerEndTileY = (playerMapY + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1) / TILE_SIZE;

	if (interactPressed)
	{
		int playerCenterX = playerMapX + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH / 2;
		int playerCenterY = playerMapY + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT / 2;
		int playerTileX = playerCenterX / TILE_SIZE;
		int playerTileY = playerCenterY / TILE_SIZE;
		int targetTileX = playerTileX;
		int targetTileY = playerTileY;

		switch (currentMoveDir)
		{
			case Direction::UP:    targetTileY--; break;
			case Direction::DOWN:  targetTileY++; break;
			case Direction::LEFT:  targetTileX--; break;
			case Direction::RIGHT: targetTileX++; break;
		}

		for (auto &event : tileMap.GetEvents())
		{
			event.UpdateActivePage(currentMapName);
			const EventPage* activePage = event.GetActivePage();
			if (!activePage)
				continue;

			if (activePage->trigger == EventTriggerType::ACTION_BUTTON)
			{
				int eventStartX = event.GetTileX();
				int eventEndX = event.GetEndTileX();
				int eventStartY = event.GetTileY();
				int eventEndY = event.GetEndTileY();

				if (targetTileX >= eventStartX && targetTileX <= eventEndX &&
					targetTileY >= eventStartY && targetTileY <= eventEndY)
				{
					char selfSwitchName[8];
					char dialogText[44]; // I need to stand up a string lookup table
					if (std::sscanf(activePage->command, "open_chest %7s %43[^\n]", selfSwitchName, dialogText) == 2)
					{
						if (!GameState::Instance().GetSelfSwitch(currentMapName, event.GetEventId(), selfSwitchName))
						{
							GameState::Instance().SetSelfSwitch(currentMapName, event.GetEventId(), selfSwitchName, true);
							dialogBox.SetText(dialogText);
						}
						event.UpdateActivePage(currentMapName);
						break;
					}
					else if (std::strncmp(activePage->command, "show_text ", 10) == 0)
					{
						dialogBox.SetText(activePage->command + 10);
						break;
					}
				}
			}
		}
	}

	for (auto &event : tileMap.GetEvents())
	{
		event.UpdateActivePage(currentMapName);

		const EventPage* activePage = event.GetActivePage();
		if (!activePage)
			continue;

		if (activePage->trigger == EventTriggerType::PLAYER_TOUCH)
		{
			int eventStartX = event.GetTileX();
			int eventEndX = event.GetEndTileX();
			int eventStartY = event.GetTileY();
			int eventEndY = event.GetEndTileY();

			if (playerStartTileX <= eventEndX && playerEndTileX >= eventStartX &&
				playerStartTileY <= eventEndY && playerEndTileY >= eventStartY)
			{
				if (playerMoving)
				{
					int tx = 0;
					int ty = 0;
					char mapName[44];
					char sfxName[55];
					if (sscanf(activePage->command, "transfer %43s %d %d", mapName, &tx, &ty) == 3)
					{
						MapTransition(mapName, tx, ty);
						break;
					}
					else if (sscanf(activePage->command, "play_sfx %54s", sfxName) == 1)
					{
						AudioManager::Instance().PlaySFX(sfxName);
						break;
					}
					else if (std::strcmp(activePage->command, "show_text") == 0 || std::strncmp(activePage->command, "show_text ", 10) == 0)
					{
						dialogBox.SetText("This is placeholder.\nPress Enter to dismiss.");
						break;
					}
				}
			}
		}
	}

	scrollX = std::clamp(playerMapX - (SCREEN_WIDTH - CHARACTER_SPRITE_WIDTH) / 2, 0, mapWidthPx - SCREEN_WIDTH);
	scrollY = std::clamp(playerMapY - (SCREEN_HEIGHT - CHARACTER_SPRITE_HEIGHT) / 2, 0, mapHeightPx - SCREEN_HEIGHT);

	tilemapChanged |= tileMap.Update();
	player.Update(playerMoving, currentMoveDir);
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
	for (auto &event : tileMap.GetEvents())
	{
		event.UpdateActivePage(currentMapName);
		const EventPage* activePage = event.GetActivePage();
		if (activePage && activePage->spriteFrame > 0 && activePage->spriteName[0] != '\0')
		{
			std::string sName = activePage->spriteName;
			auto it = eventSprites.find(sName);
			if (it != eventSprites.end() && it->second != nullptr)
			{
				int frameIndex = activePage->spriteFrame - 1;
				int drawX = event.GetMapX() - currentScrollTileX * TILE_SIZE;
				int drawY = event.GetMapY() - currentScrollTileY * TILE_SIZE;
				if (drawX >= -TILE_SIZE && drawX < VSCREEN_W && drawY >= -TILE_SIZE && drawY < VSCREEN_H)
					masked_blit(it->second, game->GetBackBuffer(), frameIndex * TILE_SIZE, 0, drawX, drawY, TILE_SIZE, TILE_SIZE);
			}
		}
	}

	player.Draw(game->GetBackBuffer(), playerMapX - currentScrollTileX * TILE_SIZE, playerMapY - currentScrollTileY * TILE_SIZE);
	tileMap.DrawUpper(game->GetBackBuffer(), tileset, currentScrollTileX, currentScrollTileY);

	if (dialogBox.IsActive())
		dialogBox.Draw(game->GetBackBuffer(), scrollX % TILE_SIZE, scrollY % TILE_SIZE);
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

	for (auto &pair : eventSprites)
	{
		if (pair.second)
			destroy_bitmap(pair.second);
	}
	eventSprites.clear();
}