#pragma once
#include <cstdint>
#include <vector>
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
		void UpdateActivePage();
		const EventPage* GetActivePage() const;

	private:
		uint16_t eventId = 0;
		int tileX = 0;
		int tileY = 0;
		int endTileX = 0;
		int endTileY = 0;
		int activePageIndex = -1;
		std::vector<EventPage> pages;
};