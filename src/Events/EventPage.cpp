#include "Events/EventPage.h"
#include "Events/Event.h"
#include "Events/EventCommand.h"
#include "Core/GameState.h"
#include "Utilities/StringUtils.h"

EventPage::EventPage(const EventPageHeader &pageHeader, std::vector<std::shared_ptr<IEventCommand>> commands
)
	: trigger(pageHeader.trigger)
	, spriteFrame(pageHeader.spriteFrame)
	, isWalkable(pageHeader.isWalkable)
	, variableThreshold(pageHeader.variableThreshold)
	, moveType(pageHeader.moveType)
	, moveSpeed(pageHeader.moveSpeed)
	, moveFrequency(pageHeader.moveFrequency)
	, movePath(StringUtils::SafeString(pageHeader.movePath, sizeof(pageHeader.movePath)))
	, switchCondition(StringUtils::SafeString(pageHeader.switchCondition, sizeof(pageHeader.switchCondition)))
	, variableCondition(StringUtils::SafeString(pageHeader.variableCondition, sizeof(pageHeader.variableCondition)))
	, selfVariableCondition(StringUtils::SafeString(pageHeader.selfVariableCondition, sizeof(pageHeader.selfVariableCondition)))
	, selfSwitchCondition(StringUtils::SafeString(pageHeader.selfSwitchCondition, sizeof(pageHeader.selfSwitchCondition)))
	, spriteName(StringUtils::SafeString(pageHeader.spriteName, sizeof(pageHeader.spriteName)))
	, commands(std::move(commands))
{
}

bool EventPage::IsActive(const std::string &mapName, uint16_t eventId) const
{
	if (!switchCondition.empty() && switchCondition != "0")
	{
		if (!GameState::Instance().GetSwitch(switchCondition))
			return false;
	}

	if (!selfSwitchCondition.empty() && selfSwitchCondition != "0")
	{
		if (!GameState::Instance().GetSelfSwitch(mapName, eventId, selfSwitchCondition))
			return false;
	}

	if (!variableCondition.empty() && variableCondition != "0")
	{
		if (GameState::Instance().GetVariable(variableCondition) < variableThreshold)
			return false;
	}

	if (!selfVariableCondition.empty() && selfVariableCondition != "0")
	{
		if (GameState::Instance().GetSelfVariable(mapName, eventId, selfVariableCondition) < variableThreshold)
			return false;
	}

	return true;
}

void EventPage::Execute(Event &event, IGameContext &context) const
{
	for (const auto &cmd : commands)
	{
		if (cmd->Execute(event, context))
			break;
	}
}
