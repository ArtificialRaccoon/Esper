#include "States/MapState.h"
#include "Core/GameState.h"
#include "Core/StringDatabase.h"
#include "Core/InteractionSystem.h"
#include "Core/TextureCache.h"
#include "Utilities/Collision.h"

void MapState::InitState(GameProcessor *game)
{
	BaseState::InitState(game);
	BUFFER = create_bitmap(VSCREEN_W, VSCREEN_H);
	PALETTE pal;
	tileset = load_bitmap(".\\TILESETS\\SHEET.bmp", pal);

	for (int i = 0; i < 256; i++)
	{
		CommonGUI::Instance().GetPalette()[i] = pal[i];
	}
	set_palette(CommonGUI::Instance().GetPalette());

	player.Load(".\\CHARS\\CHAR.bmp");
	player.SetMoving(false);
	player.SetDirection(Direction::DOWN);

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
	activeEventId = 0;
	parallelTasks.clear();
	tileMap.Load(".\\MAPS\\" + mapName + ".bin");

	TextureCache::Instance().Clear();
	for (auto &event : tileMap.GetEvents())
	{
		event->UpdateActivePage(currentMapName);
		const EventPage *page = event->GetActivePage();
		if (page && !page->GetSpriteName().empty())
			TextureCache::Instance().Get(page->GetSpriteName());
	}

	mapWidthPx = tileMap.GetLayers().empty() ? 0 : tileMap.GetLayers()[0].width * TILE_SIZE;
	mapHeightPx = tileMap.GetLayers().empty() ? 0 : tileMap.GetLayers()[0].height * TILE_SIZE;

	player.SetTilePosition(targetTileX, targetTileY);

	scrollX = std::clamp(player.GetMapX() - (SCREEN_WIDTH - CHARACTER_SPRITE_WIDTH) / 2, 0, std::max(0, mapWidthPx - SCREEN_WIDTH));
	scrollY = std::clamp(player.GetMapY() - (SCREEN_HEIGHT - CHARACTER_SPRITE_HEIGHT) / 2, 0, std::max(0, mapHeightPx - SCREEN_HEIGHT));

	prevScrollTileX = -1;
	prevScrollTileY = -1;
	tilemapChanged = true;
}

bool MapState::CheckEventCollision(int targetX, int targetY)
{
	Rect targetRect(targetX, targetX + CHARACTER_HITBOX_WIDTH - 1, targetY, targetY + CHARACTER_HITBOX_HEIGHT - 1);
	Rect curRect(player.GetMapX() + CHARACTER_HITBOX_X_OFFSET,
	             player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1,
	             player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET,
	             player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1);

	for (auto &event : tileMap.GetEvents())
	{
		const EventPage* activePage = event->GetActivePage();
		if (activePage && activePage->GetIsWalkable() == 0)
		{
			Rect evRect = event->GetHitbox();

			if (Collision::RectOverlaps(targetRect, evRect))
			{
				if (!Collision::RectOverlaps(curRect, evRect))
					return true;
				
				int curOverlapArea = Collision::GetOverlapArea(curRect, evRect);
				int targetOverlapArea = Collision::GetOverlapArea(targetRect, evRect);
				if (targetOverlapArea >= curOverlapArea)
					return true;
			}
		}
	}
	return false;
}

bool MapState::IsWalkable(int targetMapX, int targetMapY)
{
	int hitboxX = targetMapX + CHARACTER_HITBOX_X_OFFSET;
	int hitboxY = targetMapY + CHARACTER_HITBOX_Y_OFFSET;
	if (tileMap.CheckCollision(hitboxX, hitboxY, CHARACTER_HITBOX_WIDTH, CHARACTER_HITBOX_HEIGHT))
		return false;
	if (CheckEventCollision(hitboxX, hitboxY))
		return false;
	return true;
}

void MapState::AcquireInput(GameProcessor *game)
{
	for (auto &event : tileMap.GetEvents())
	{
		event->UpdateActivePage(currentMapName);
	}

	interactPressed = false;
	if (InputManager::Instance().IsKeyPressed(KEY_ENTER) || InputManager::Instance().IsKeyPressed(KEY_SPACE))
		interactPressed = true;

	if (IsFading() || choiceBox.IsActive() || dialogBox.IsActive() || interpreter.IsActive())
		return;

	player.ProcessMovementInput(mapWidthPx, mapHeightPx, [this](int x, int y) { return IsWalkable(x, y); });
	if (InputManager::Instance().IsKeyPressed(KEY_ESC))
		game->Quit();
}

void MapState::ProcessInput(GameProcessor *game)
{
	for (auto &event : tileMap.GetEvents())
	{
		event->UpdateActivePage(currentMapName);
	}

	if (interpreter.IsActive())
		interpreter.Update(*this);

	UpdateParallelTasks();

	if (IsFading())
		return;

	if (choiceBox.IsActive())
	{
		choiceBox.Update();
		if (InputManager::Instance().IsKeyPressed(KEY_UP))
			choiceBox.SelectPrevious();
		else if (InputManager::Instance().IsKeyPressed(KEY_DOWN))
			choiceBox.SelectNext();

		if (interactPressed)
			choiceBox.Confirm();

		return;
	}

	if (dialogBox.IsActive())
	{
		dialogBox.Update();
		if (interactPressed)
			dialogBox.Advance();

		if (!dialogBox.IsActive() && activeEventId != 0 && !interpreter.IsActive() && !choiceBox.IsActive())
		{
			if (Event *ev = GetEventById(activeEventId))
				ev->OnInteractionEnd();
			activeEventId = 0;
		}
		return;
	}

	if (!interpreter.IsActive() && !dialogBox.IsActive() && !choiceBox.IsActive() && activeEventId != 0)
	{
		if (Event *ev = GetEventById(activeEventId))
			ev->OnInteractionEnd();
		activeEventId = 0;
	}

	if (!interpreter.IsActive() && !dialogBox.IsActive() && !choiceBox.IsActive())
	{
		for (auto &event : tileMap.GetEvents())
		{
			const EventPage *activePage = event->GetActivePage();
			if (activePage && activePage->GetTrigger() == EventTriggerType::AUTORUN)
			{
				interpreter.Start(activePage->GetCommands(), event->GetEventId(), *this);
				break;
			}
		}
	}

	if (!interpreter.IsActive() && !dialogBox.IsActive() && !choiceBox.IsActive())
	{
		if (interactPressed)
			InteractionSystem::ProcessAction(player, tileMap, currentMapName, *this, activeEventId);
		InteractionSystem::ProcessTouch(player, tileMap, currentMapName, *this);
	}

	for (auto &event : tileMap.GetEvents())
	{
		event->Update(tileMap, player.GetMapX(), player.GetMapY(), tileMap.GetEvents(), dialogBox.IsActive() || choiceBox.IsActive(), *this);
	}

	if (player.HasActiveMoveRoute())
	{
		player.UpdateMoveRouteStep([this](int tx, int ty) {
			return IsWalkable(tx * TILE_SIZE, ty * TILE_SIZE);
		});
	}

	scrollX = std::clamp(player.GetMapX() - (SCREEN_WIDTH - CHARACTER_SPRITE_WIDTH) / 2, 0, std::max(0, mapWidthPx - SCREEN_WIDTH));
	scrollY = std::clamp(player.GetMapY() - (SCREEN_HEIGHT - CHARACTER_SPRITE_HEIGHT) / 2, 0, std::max(0, mapHeightPx - SCREEN_HEIGHT));

	tilemapChanged |= tileMap.Update();
	player.UpdateAnimation();
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
	renderSystem.DrawEntities(game->GetBackBuffer(), player, tileMap, currentMapName, currentScrollTileX, currentScrollTileY);
	tileMap.DrawUpper(game->GetBackBuffer(), tileset, currentScrollTileX, currentScrollTileY);
	if (dialogBox.IsActive())
		dialogBox.Draw(game->GetBackBuffer(), scrollX % TILE_SIZE, scrollY % TILE_SIZE);
	if (choiceBox.IsActive())
		choiceBox.Draw(game->GetBackBuffer(), scrollX % TILE_SIZE, scrollY % TILE_SIZE);
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

	parallelTasks.clear();
	TextureCache::Instance().Clear();
}

bool MapState::IsDialogActive() const
{
	return dialogBox.IsActive();
}

bool MapState::IsDialogTyping() const
{
	return dialogBox.IsTyping();
}

void MapState::CloseDialog()
{
	dialogBox.Close();
}

bool MapState::IsChoiceActive() const
{
	return choiceBox.IsActive();
}

bool MapState::IsFading() const
{
	return gameRef ? gameRef->IsFading() : false;
}

Actor* MapState::GetActorById(int id)
{
	if (id == -1 || id == PLAYER_ACTOR_ID)
		return &player;

	for (auto &event : tileMap.GetEvents())
	{
		if (event->GetEventId() == id)
			return event->AsActor();
	}

	return nullptr;
}

Event* MapState::GetEventById(int id)
{
	for (auto &event : tileMap.GetEvents())
	{
		if (event->GetEventId() == id)
			return event.get();
	}

	return nullptr;
}

void MapState::PlayBGM(const std::string &bgmName)
{
	AudioManager::Instance().PlayMusic(bgmName);
}

void MapState::FadeOut(int speed)
{
	if (gameRef)
		gameRef->FadeOut(speed);
}

void MapState::FadeIn(int speed)
{
	if (gameRef)
		gameRef->FadeIn(speed);
}

void MapState::ShowText(const std::string &text)
{
	dialogBox.SetText(text);
}

void MapState::ShowChoices(const std::vector<std::string> &choices)
{
	choiceBox.SetChoices(choices);
}

int MapState::GetSelectedChoice() const
{
	return choiceBox.GetSelectedIndex();
}

void MapState::TransferPlayer(const std::string &mapName, int tileX, int tileY)
{
	MapTransition(mapName, tileX, tileY);

	PALETTE blackPal;
	memset(blackPal, 0, sizeof(PALETTE));
	set_palette(blackPal);

	if (gameRef)
	{
		FrameRender(gameRef);
		gameRef->FlipPages();
		gameRef->FadeIn(DEFAULT_FADE_SPEED);
	}
}

void MapState::StartEventScript(const std::vector<std::shared_ptr<IEventCommand>> &commands, uint16_t eventId)
{
	activeEventId = eventId;
	interpreter.Start(commands, eventId, *this);
}

void MapState::UpdateParallelTasks()
{
	std::vector<uint16_t> activeParallelIds;

	for (auto &event : tileMap.GetEvents())
	{
		const EventPage *activePage = event->GetActivePage();
		if (activePage && activePage->GetTrigger() == EventTriggerType::PARALLEL)
		{
			uint16_t evId = event->GetEventId();
			activeParallelIds.push_back(evId);

			auto it = parallelTasks.find(evId);
			if (it == parallelTasks.end() || !it->second.IsActive())
			{
				parallelTasks[evId].Start(activePage->GetCommands(), evId, *this, true);
			}
			else
			{
				it->second.Update(*this);
			}
		}
	}

	for (auto it = parallelTasks.begin(); it != parallelTasks.end(); )
	{
		if (std::find(activeParallelIds.begin(), activeParallelIds.end(), it->first) == activeParallelIds.end())
			it = parallelTasks.erase(it);
		else
			++it;
	}
}