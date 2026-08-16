#include "Events/EventInterpreter.h"
#include "Core/IGameContext.h"
#include "Core/Actor.h"
#include "Events/Event.h"

void EventInterpreter::Start(const std::vector<std::shared_ptr<IEventCommand>> &cmds, uint16_t triggeringEventId, IGameContext &context, bool loop)
{
	if (cmds.empty())
	{
		isActive = false;
		isLooping = false;
		return;
	}

	rootCommands = cmds;
	frameStack.clear();
	frameStack.push_back(ExecutionFrame{ cmds, 0 });
	isActive = true;
	isLooping = loop;
	currentWaitState = CommandResult::Continue();
	triggerId = triggeringEventId;

	Update(context);
}

void EventInterpreter::Clear()
{
	isActive = false;
	isLooping = false;
	rootCommands.clear();
	frameStack.clear();
	currentWaitState = CommandResult::Continue();
	triggerId = 0;
}

void EventInterpreter::Update(IGameContext &context)
{
	if (!isActive || context.IsFading())
		return;

	if (currentWaitState.status == CommandStatus::WAIT_FOR_TEXT)
	{
		if (!context.IsDialogActive())
		{
			currentWaitState = CommandResult::Continue();
		}
		else if (!context.IsDialogTyping())
		{
			bool nextIsChoices = false;
			if (!frameStack.empty())
			{
				const auto &frame = frameStack.back();
				if (frame.currentCmdIndex < frame.commands.size())
				{
					if (std::dynamic_pointer_cast<ShowChoicesCommand>(frame.commands[frame.currentCmdIndex]))
					{
						nextIsChoices = true;
					}
				}
			}

			if (nextIsChoices)
			{
				currentWaitState = CommandResult::Continue();
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
	else if (currentWaitState.status == CommandStatus::WAIT_FOR_CHOICE)
	{
		if (context.IsChoiceActive())
			return;

		int selected = context.GetSelectedChoice();
		bool subCmdHasText = false;
		if (!frameStack.empty() && frameStack.back().currentCmdIndex > 0)
		{
			auto lastCmd = frameStack.back().commands[frameStack.back().currentCmdIndex - 1];
			auto choiceCmd = std::dynamic_pointer_cast<ShowChoicesCommand>(lastCmd);
			if (choiceCmd)
			{
				const auto &options = choiceCmd->GetOptions();
				if (selected >= 0 && selected < static_cast<int>(options.size()))
				{
					const auto &subCmds = options[selected].commands;
					if (!subCmds.empty())
					{
						frameStack.push_back(ExecutionFrame{ subCmds, 0 });
						for (const auto &sc : subCmds)
						{
							if (std::dynamic_pointer_cast<ShowTextCommand>(sc))
							{
								subCmdHasText = true;
								break;
							}
						}
					}
				}
			}
		}

		if (!subCmdHasText && context.IsDialogActive())
		{
			context.CloseDialog();
		}

		currentWaitState = CommandResult::Continue();
	}
	else if (currentWaitState.status == CommandStatus::WAIT_FRAMES)
	{
		currentWaitState.data--;
		if (currentWaitState.data > 0)
			return;

		currentWaitState = CommandResult::Continue();
	}
	else if (currentWaitState.status == CommandStatus::WAIT_FOR_ACTOR)
	{
		Actor *actor = context.GetActorById(currentWaitState.data);
		if (actor && actor->HasActiveMoveRoute())
			return;

		currentWaitState = CommandResult::Continue();
	}
	else if (currentWaitState.status == CommandStatus::YIELD)
	{
		currentWaitState = CommandResult::Continue();
	}

	while (!frameStack.empty())
	{
		ExecutionFrame &frame = frameStack.back();
		if (frame.currentCmdIndex >= frame.commands.size())
		{
			frameStack.pop_back();
			continue;
		}

		auto cmd = frame.commands[frame.currentCmdIndex];
		if (!cmd)
		{
			frame.currentCmdIndex++;
			continue;
		}

		ExecutionContext ctx{
			context,
			context.GetEventById(triggerId),
			triggerId
		};

		CommandResult result = cmd->Execute(ctx);
		frame.currentCmdIndex++;

		switch (result.status)
		{
			case CommandStatus::CONTINUE:
				break;

			case CommandStatus::YIELD:
			case CommandStatus::WAIT_FRAMES:
			case CommandStatus::WAIT_FOR_ACTOR:
			case CommandStatus::WAIT_FOR_CHOICE:
			case CommandStatus::WAIT_FOR_TEXT:
				currentWaitState = result;
				return;

			case CommandStatus::TERMINATE:
				Clear();
				return;
		}

		if (context.IsChoiceActive() || context.IsFading())
			return;
	}

	if (isLooping)
	{
		frameStack.clear();
		frameStack.push_back(ExecutionFrame{ rootCommands, 0 });
		currentWaitState = CommandResult::Yield();
		return;
	}

	Clear();
}
