#include "Events/EventCommand.h"
#include "Events/Event.h"
#include "Events/EventSerialization.h"
#include "Core/GameState.h"
#include "Core/StringDatabase.h"
#include "Utilities/AudioManager.h"

// ShowTextCommand
ShowTextCommand::ShowTextCommand(uint16_t stringId)
	: stringId(stringId)
{
}

bool ShowTextCommand::Execute(Event &event, IGameContext &context)
{
	std::string dialogText = StringDatabase::Instance().GetString(stringId);
	context.ShowText(dialogText);
	return false;
}

// ControlSelfSwitchCommand
ControlSelfSwitchCommand::ControlSelfSwitchCommand(const std::string &selfSwitch, bool value)
	: selfSwitch(selfSwitch)
	, value(value)
{
}

bool ControlSelfSwitchCommand::Execute(Event &event, IGameContext &context)
{
	GameState::Instance().SetSelfSwitch(context.GetCurrentMapName(), event.GetEventId(), selfSwitch, value);
	event.UpdateActivePage(context.GetCurrentMapName());
	return false;
}

// ControlSelfVarCommand
ControlSelfVarCommand::ControlSelfVarCommand(const std::string &selfVar, uint8_t op, int16_t val)
	: selfVar(selfVar)
	, op(op)
	, val(val)
{
}

bool ControlSelfVarCommand::Execute(Event &event, IGameContext &context)
{
	int curVal = GameState::Instance().GetSelfVariable(context.GetCurrentMapName(), event.GetEventId(), selfVar);
	int newVal = (op == 1) ? (curVal + val) : val;
	GameState::Instance().SetSelfVariable(context.GetCurrentMapName(), event.GetEventId(), selfVar, newVal);
	event.UpdateActivePage(context.GetCurrentMapName());
	return false;
}

// ControlVarCommand
ControlVarCommand::ControlVarCommand(const std::string &varName, uint8_t op, int16_t val)
	: varName(varName)
	, op(op)
	, val(val)
{
}

bool ControlVarCommand::Execute(Event &event, IGameContext &context)
{
	int curVal = GameState::Instance().GetVariable(varName);
	int newVal = (op == 1) ? (curVal + val) : val;
	GameState::Instance().SetVariable(varName, newVal);
	event.UpdateActivePage(context.GetCurrentMapName());
	return false;
}

// PlaySFXCommand
PlaySFXCommand::PlaySFXCommand(const std::string &sfxName)
	: sfxName(sfxName)
{
}

bool PlaySFXCommand::Execute(Event &event, IGameContext &context)
{
	AudioManager::Instance().PlaySFX(sfxName);
	return false;
}

// TransferPlayerCommand
TransferPlayerCommand::TransferPlayerCommand(const std::string &mapName, int16_t tileX, int16_t tileY)
	: mapName(mapName)
	, tileX(tileX)
	, tileY(tileY)
{
}

bool TransferPlayerCommand::Execute(Event &event, IGameContext &context)
{
	context.TransferPlayer(mapName, tileX, tileY);
	return true;
}

// Factory
std::shared_ptr<IEventCommand> CreateEventCommand(const EventCommand &packedCmd)
{
	switch (packedCmd.type)
	{
		case CommandType::SHOW_TEXT:
			return std::make_shared<ShowTextCommand>(static_cast<uint16_t>(packedCmd.val));
		case CommandType::CONTROL_SELF_SWITCH:
			return std::make_shared<ControlSelfSwitchCommand>(packedCmd.strParam1, packedCmd.op != 0);
		case CommandType::CONTROL_SELF_VAR:
			return std::make_shared<ControlSelfVarCommand>(packedCmd.strParam1, packedCmd.op, packedCmd.val);
		case CommandType::CONTROL_VAR:
			return std::make_shared<ControlVarCommand>(packedCmd.strParam1, packedCmd.op, packedCmd.val);
		case CommandType::PLAY_SFX:
			return std::make_shared<PlaySFXCommand>(packedCmd.strParam1);
		case CommandType::TRANSFER_PLAYER:
			return std::make_shared<TransferPlayerCommand>(packedCmd.strParam1, packedCmd.val, packedCmd.extraVal);
		default:
			break;
	}
	return nullptr;
}
