#include "Events/Event.h"
#include "Events/EventPage.h"

Event::Event(uint16_t id)
	: eventId(id)
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
		activePageIndex = newActiveIndex;
		OnPageChanged();
	}
}

const EventPage* Event::GetActivePage() const
{
	if (activePageIndex >= 0 && activePageIndex < static_cast<int>(pages.size()))
		return &pages[activePageIndex];
	return nullptr;
}