#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <allegro.h>
#include "Core/IGameContext.h"
#include "Core/GameDefines.h"
#include "Events/EventPage.h"
#include "Utilities/Collision.h"
#include "Core/IRenderable.h"

class TileMap;

class Event : public IRenderable
{
	public:
		Event() = default;
		Event(uint16_t id, int tileX, int tileY, int endTileX = -1, int endTileY = -1);
		virtual ~Event() override = default;

		uint16_t GetEventId() const { return eventId; }
		virtual int GetTileX() const { return tileX; }
		virtual int GetTileY() const { return tileY; }
		virtual int GetEndTileX() const = 0;
		virtual int GetEndTileY() const = 0;
		virtual int GetMapX() const = 0;
		virtual int GetMapY() const = 0;

		void AddPage(const EventPage& page) { pages.push_back(page); }
		void UpdateActivePage(const std::string &mapName);
		const EventPage* GetActivePage() const;
		const std::vector<EventPage>& GetPages() const { return pages; }

		virtual Rect GetHitbox() const = 0;
		virtual bool CollidesWith(const Rect &playerRect) const = 0;

		void ExecutePageCommands(const EventPage *page, IGameContext &context);

	public:
		int GetSortY() const override { return GetMapY(); }
		void Draw(BITMAP *dest, int scrollTileX, int scrollTileY) const override = 0;

	protected:
		virtual void OnPageChanged(bool wasActor) {}

		uint16_t eventId = 0;
		int tileX = 0;
		int tileY = 0;
		int endTileX = 0;
		int endTileY = 0;
		int activePageIndex = -1;
		std::vector<EventPage> pages;
};