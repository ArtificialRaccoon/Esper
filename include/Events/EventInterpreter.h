#pragma once
#include <vector>
#include <memory>
#include "Events/EventCommand.h"

class IGameContext;

class EventInterpreter
{
	public:
		EventInterpreter() = default;
		~EventInterpreter() = default;

		void Start(const std::vector<std::shared_ptr<IEventCommand>> &cmds, uint16_t triggeringEventId, IGameContext &context);
		void Update(IGameContext &context);
		bool IsActive() const { return isActive; }
		void Clear();

	private:
		bool isActive = false;
		std::vector<std::shared_ptr<IEventCommand>> commands;
		size_t currentCmdIndex = 0;
		CommandResult currentWaitState = CommandResult::Continue();
		uint16_t triggerId = 0;
};

