#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <allegro.h>
#include "Core/GameDefines.h"

enum class EventTriggerType : uint8_t
{
	ACTION_BUTTON,
	PLAYER_TOUCH,
	AUTORUN,
	NONE
};

struct __attribute__((packed)) EventPage
{
	EventTriggerType trigger = EventTriggerType::NONE;
	uint8_t spriteFrame = 0;
	uint8_t isWalkable = 1;
	int16_t variableThreshold = 0;
	char switchCondition[24] = { 0 };
	char variableCondition[24] = { 0 };
	char selfSwitchCondition[8] = { 0 };
	char spriteName[8] = { 0 };
	char command[64] = { 0 };
};

struct __attribute__((packed)) GameEvent
{
	uint16_t eventId;
	int16_t tileX;
	int16_t tileY;
	int16_t endTileX;
	int16_t endTileY;
	uint16_t pageCount;
};

class Event
{
	public:
		Event() = default;
		Event(uint16_t id, int tileX, int tileY, int endTileX = -1, int endTileY = -1);
		~Event() = default;
		uint16_t GetEventId() const { return eventId; }
		int GetTileX() const { return tileX; }
		int GetTileY() const { return tileY; }
		int GetEndTileX() const { return endTileX; }
		int GetEndTileY() const { return endTileY; }
		int GetMapX() const { return tileX * TILE_SIZE; }
		int GetMapY() const { return tileY * TILE_SIZE; }
		void AddPage(const EventPage& page) { pages.push_back(page); }
		void UpdateActivePage(const std::string &mapName);
		const EventPage* GetActivePage() const;
		const std::vector<EventPage>& GetPages() const { return pages; }

	private:
		uint16_t eventId = 0;
		int tileX = 0;
		int tileY = 0;
		int endTileX = 0;
		int endTileY = 0;
		int activePageIndex = -1;
		std::vector<EventPage> pages;
};