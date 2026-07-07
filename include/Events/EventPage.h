#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Events/EventSerialization.h"

class Event;
class IGameContext;
class IEventCommand;

class EventPage
{
	public:
		EventPage() = default;
		EventPage(const EventPageHeader &pageHeader, std::vector<std::shared_ptr<IEventCommand>> commands = {});
		EventMoveType GetMoveType() const { return moveType; }
		EventTriggerType GetTrigger() const { return trigger; }
		uint8_t GetSpriteFrame() const { return spriteFrame; }
		uint8_t GetIsWalkable() const { return isWalkable; }				
		uint8_t GetMoveSpeed() const { return moveSpeed; }
		uint8_t GetMoveFrequency() const { return moveFrequency; }
		int16_t GetVariableThreshold() const { return variableThreshold; }
		const std::string& GetMovePath() const { return movePath; }
		const std::string& GetSwitchCondition() const { return switchCondition; }
		const std::string& GetVariableCondition() const { return variableCondition; }
		const std::string& GetSelfVariableCondition() const { return selfVariableCondition; }
		const std::string& GetSelfSwitchCondition() const { return selfSwitchCondition; }
		const std::string& GetSpriteName() const { return spriteName; }
		const std::vector<std::shared_ptr<IEventCommand>>& GetCommands() const { return commands; }
		bool IsActive(const std::string &mapName, uint16_t eventId) const;
		void Execute(Event &event, IGameContext &context) const;

	private:
		EventMoveType moveType = EventMoveType::FIXED;
		EventTriggerType trigger = EventTriggerType::NONE;
		uint8_t spriteFrame = 0;
		uint8_t isWalkable = 1;		
		uint8_t moveSpeed = 1;
		uint8_t moveFrequency = 30;
		int16_t variableThreshold = 0;
		std::string movePath;
		std::string switchCondition;
		std::string variableCondition;
		std::string selfVariableCondition;
		std::string selfSwitchCondition;
		std::string spriteName;
		std::vector<std::shared_ptr<IEventCommand>> commands;
};
