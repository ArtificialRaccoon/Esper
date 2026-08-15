#include "Core/InteractionSystem.h"
#include "Core/Player.h"
#include "Core/TileMap.h"
#include "Core/IGameContext.h"
#include "Utilities/Collision.h"
#include "Events/Event.h"
#include "Events/EventPage.h"
#include "Events/EventSerialization.h"
#include <algorithm>

void InteractionSystem::ProcessAction(Player &player, TileMap &tileMap, const std::string &currentMapName, IGameContext &context, uint16_t &activeEventId)
{
	int playerCenterX = player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH / 2;
	int playerCenterY = player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT / 2;

	Rect playerHitbox(
		player.GetMapX() + CHARACTER_HITBOX_X_OFFSET,
		player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1,
		player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET,
		player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1
	);

	Rect interactRect = playerHitbox;
	switch (player.GetDirection())
	{
		case Direction::UP:
			interactRect.top = interactRect.top - TILE_SIZE;
			interactRect.bottom = playerHitbox.top - 1;
			break;
		case Direction::DOWN:
			interactRect.top = playerHitbox.bottom + 1;
			interactRect.bottom = interactRect.bottom + TILE_SIZE;
			break;
		case Direction::LEFT:
			interactRect.left = interactRect.left - TILE_SIZE;
			interactRect.right = playerHitbox.left - 1;
			break;
		case Direction::RIGHT:
			interactRect.left = playerHitbox.right + 1;
			interactRect.right = interactRect.right + TILE_SIZE;
			break;
	}

	int targetTileX = (player.GetMapX() + CHARACTER_HITBOX_X_OFFSET) / TILE_SIZE;
	int targetTileY = (player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET) / TILE_SIZE;
	switch (player.GetDirection())
	{
		case Direction::UP:
			targetTileY--;
			break;
		case Direction::DOWN:
			targetTileY++;
			break;
		case Direction::LEFT:
			targetTileX--;
			break;
		case Direction::RIGHT:
			targetTileX++;
			break;
	}

	for (auto &event : tileMap.GetEvents())
	{
		const EventPage *activePage = event->GetActivePage();
		if (!activePage)
			continue;

		if (activePage->GetTrigger() == EventTriggerType::ACTION_BUTTON)
		{
			bool isHit = false;

			Rect evRect = event->GetHitbox();
			if (Collision::RectOverlaps(interactRect, evRect))
				isHit = true;
			else
			{
				int eventStartX = event->GetTileX();
				int eventEndX = event->GetEndTileX();
				int eventStartY = event->GetTileY();
				int eventEndY = event->GetEndTileY();
				if (targetTileX >= eventStartX && targetTileX <= eventEndX && targetTileY >= eventStartY && targetTileY <= eventEndY)
					isHit = true;
			}

			if (isHit)
			{
				event->OnInteractionStart(player, context);
				activeEventId = event->GetEventId();
				context.StartEventScript(activePage->GetCommands(), event->GetEventId());
				break;
			}
		}
	}
}

void InteractionSystem::ProcessTouch(Player &player, TileMap &tileMap, const std::string &currentMapName, IGameContext &context)
{
	int playerStartTileX = (player.GetMapX() + CHARACTER_HITBOX_X_OFFSET) / TILE_SIZE;
	int playerEndTileX = (player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1) / TILE_SIZE;
	int playerStartTileY = (player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET) / TILE_SIZE;
	int playerEndTileY = (player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1) / TILE_SIZE;

	for (auto &event : tileMap.GetEvents())
	{
		const EventPage *activePage = event->GetActivePage();
		if (!activePage)
			continue;

		if (activePage->GetTrigger() == EventTriggerType::PLAYER_TOUCH ||
		    activePage->GetTrigger() == EventTriggerType::EVENT_TOUCH)
		{
			int eventStartX = event->GetTileX();
			int eventEndX = event->GetEndTileX();
			int eventStartY = event->GetTileY();
			int eventEndY = event->GetEndTileY();

			bool isColliding = (playerStartTileX <= eventEndX && playerEndTileX >= eventStartX &&
			                    playerStartTileY <= eventEndY && playerEndTileY >= eventStartY);

			if (!isColliding)
			{
				Rect playerHitbox(
					player.GetMapX() + CHARACTER_HITBOX_X_OFFSET,
					player.GetMapX() + CHARACTER_HITBOX_X_OFFSET + CHARACTER_HITBOX_WIDTH - 1,
					player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET,
					player.GetMapY() + CHARACTER_HITBOX_Y_OFFSET + CHARACTER_HITBOX_HEIGHT - 1
				);
				if (Collision::RectOverlaps(playerHitbox, event->GetHitbox()))
					isColliding = true;
			}

			if (isColliding && player.IsMoving())
			{
				event->OnInteractionStart(player, context);
				context.StartEventScript(activePage->GetCommands(), event->GetEventId());
				break;
			}
		}
	}
}
