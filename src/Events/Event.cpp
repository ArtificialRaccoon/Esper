#include "Events/Event.h"

Event::Event(uint16_t id, int tileX, int tileY, int endTileX, int endTileY)
	: eventId(id)
	, tileX(tileX)
	, tileY(tileY)
	, endTileX(endTileX == -1 ? tileX : endTileX)
	, endTileY(endTileY == -1 ? tileY : endTileY)
	, activePageIndex(-1)
{
}

void Event::UpdateActivePage()
{
	int newActiveIndex = -1;
	if (!pages.empty())
		newActiveIndex = static_cast<int>(pages.size()) - 1;

	if (newActiveIndex != activePageIndex)
		activePageIndex = newActiveIndex;
}

const EventPage* Event::GetActivePage() const
{
	if (activePageIndex >= 0 && activePageIndex < static_cast<int>(pages.size()))
		return &pages[activePageIndex];
	return nullptr;
}