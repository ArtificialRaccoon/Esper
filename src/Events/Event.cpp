#include <typeinfo>
#include "Events/Event.h"
#include "Events/EventPage.h"
#include "Events/ActorEvent.h"

Event::Event(uint16_t id, int tileX, int tileY, int endTileX, int endTileY)
	: eventId(id)
	, tileX(tileX)
	, tileY(tileY)
	, endTileX(endTileX == -1 ? tileX : endTileX)
	, endTileY(endTileY == -1 ? tileY : endTileY)
	, activePageIndex(-1)
{
}

void Event::UpdateActivePage(const std::string &mapName)
{
	int newActiveIndex = -1;
	for (int i = static_cast<int>(pages.size()) - 1; i >= 0; i--)
	{
		if (pages[i].IsActive(mapName, eventId))
		{
			newActiveIndex = i;
			break;
		}
	}

	if (newActiveIndex != activePageIndex)
	{
		bool wasActor = (typeid(*this) == typeid(ActorEvent));
		activePageIndex = newActiveIndex;
		OnPageChanged(wasActor);
	}
}

const EventPage* Event::GetActivePage() const
{
	if (activePageIndex >= 0 && activePageIndex < static_cast<int>(pages.size()))
		return &pages[activePageIndex];
	return nullptr;
}

void Event::ExecutePageCommands(const EventPage *page, IGameContext &context)
{
	if (page)
		page->Execute(*this, context);
}