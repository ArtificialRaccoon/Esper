#include "Events/Event.h"
#include "Core/GameState.h"

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
		const auto &page = pages[i];
		bool conditionMet = true;

		if (page.switchCondition[0] != '\0' && page.switchCondition[0] != '0' && !GameState::Instance().GetSwitch(page.switchCondition))
			conditionMet = false;

		if (conditionMet && page.selfSwitchCondition[0] != '\0' && page.selfSwitchCondition[0] != '0' && !GameState::Instance().GetSelfSwitch(mapName, eventId, page.selfSwitchCondition))
			conditionMet = false;

		if (conditionMet && page.variableCondition[0] != '\0' && page.variableCondition[0] != '0' && GameState::Instance().GetVariable(page.variableCondition) < page.variableThreshold)
			conditionMet = false;

		if (conditionMet)
		{
			newActiveIndex = i;
			break;
		}
	}

	if (newActiveIndex != activePageIndex)
		activePageIndex = newActiveIndex;
}

const EventPage* Event::GetActivePage() const
{
	if (activePageIndex >= 0 && activePageIndex < static_cast<int>(pages.size()))
		return &pages[activePageIndex];
	return nullptr;
}