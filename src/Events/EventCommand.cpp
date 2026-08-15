#include "Events/EventCommand.h"
#include "Events/Event.h"
#include "Events/EventSerialization.h"
#include "Core/GameState.h"
#include "Core/StringDatabase.h"
#include "Core/PathDatabase.h"
#include "Core/Actor.h"
#include "Core/IGameContext.h"
#include "Utilities/AudioManager.h"
#include "Utilities/StringUtils.h"

// ShowTextCommand
ShowTextCommand::ShowTextCommand(uint16_t stringId)
	: stringId(stringId)
{
}

CommandResult ShowTextCommand::Execute(ExecutionContext &ctx)
{
	std::string dialogText = StringDatabase::Instance().GetString(stringId);
	ctx.context.ShowText(dialogText);
	return CommandResult::Yield();
}

// ControlSelfSwitchCommand
ControlSelfSwitchCommand::ControlSelfSwitchCommand(const std::string &selfSwitch, bool value)
	: selfSwitch(selfSwitch)
	, value(value)
{
}

CommandResult ControlSelfSwitchCommand::Execute(ExecutionContext &ctx)
{
	uint16_t evId = ctx.event ? ctx.event->GetEventId() : ctx.triggeringEventId;
	GameState::Instance().SetSelfSwitch(ctx.context.GetCurrentMapName(), evId, selfSwitch, value);
	if (ctx.event)
		ctx.event->UpdateActivePage(ctx.context.GetCurrentMapName());
	return CommandResult::Continue();
}

// ControlSelfVarCommand
ControlSelfVarCommand::ControlSelfVarCommand(const std::string &selfVar, uint8_t op, int16_t val)
	: selfVar(selfVar)
	, op(op)
	, val(val)
{
}

CommandResult ControlSelfVarCommand::Execute(ExecutionContext &ctx)
{
	uint16_t evId = ctx.event ? ctx.event->GetEventId() : ctx.triggeringEventId;
	int curVal = GameState::Instance().GetSelfVariable(ctx.context.GetCurrentMapName(), evId, selfVar);
	int newVal = (op == 1) ? (curVal + val) : val;
	GameState::Instance().SetSelfVariable(ctx.context.GetCurrentMapName(), evId, selfVar, newVal);
	if (ctx.event)
		ctx.event->UpdateActivePage(ctx.context.GetCurrentMapName());
	return CommandResult::Continue();
}

// ControlVarCommand
ControlVarCommand::ControlVarCommand(const std::string &varName, uint8_t op, int16_t val)
	: varName(varName)
	, op(op)
	, val(val)
{
}

CommandResult ControlVarCommand::Execute(ExecutionContext &ctx)
{
	int curVal = GameState::Instance().GetVariable(varName);
	int newVal = (op == 1) ? (curVal + val) : val;
	GameState::Instance().SetVariable(varName, newVal);
	if (ctx.event)
		ctx.event->UpdateActivePage(ctx.context.GetCurrentMapName());
	return CommandResult::Continue();
}

// PlaySFXCommand
PlaySFXCommand::PlaySFXCommand(const std::string &sfxName)
	: sfxName(sfxName)
{
}

CommandResult PlaySFXCommand::Execute(ExecutionContext &ctx)
{
	AudioManager::Instance().PlaySFX(sfxName);
	return CommandResult::Continue();
}

// TransferPlayerCommand
TransferPlayerCommand::TransferPlayerCommand(const std::string &mapName, int16_t tileX, int16_t tileY)
	: mapName(mapName)
	, tileX(tileX)
	, tileY(tileY)
{
}

CommandResult TransferPlayerCommand::Execute(ExecutionContext &ctx)
{
	ctx.context.TransferPlayer(mapName, tileX, tileY);
	return CommandResult::Terminate();
}

// SetMoveRouteCommand
SetMoveRouteCommand::SetMoveRouteCommand(int16_t targetId, uint16_t pathId, bool bypassCollision)
	: targetId(targetId)
	, pathId(pathId)
	, bypassCollision(bypassCollision)
{
}

CommandResult SetMoveRouteCommand::Execute(ExecutionContext &ctx)
{
	Actor *actor = ctx.context.GetActorById(targetId);
	if (actor)
	{
		std::string pathStr = PathDatabase::Instance().GetPath(pathId);
		std::vector<Direction> steps = StringUtils::ParseMovePath(pathStr);
		actor->QueueMoveRoute(steps, bypassCollision);
	}
	return CommandResult::Continue();
}

// WaitCommand
WaitCommand::WaitCommand(int16_t frames)
	: frames(frames)
{
}

CommandResult WaitCommand::Execute(ExecutionContext &ctx)
{
	return CommandResult::WaitFrames(frames);
}

// WaitForMovementCommand
WaitForMovementCommand::WaitForMovementCommand(int16_t targetId)
	: targetId(targetId)
{
}

CommandResult WaitForMovementCommand::Execute(ExecutionContext &ctx)
{
	return CommandResult::WaitForActor(targetId);
}

// FadeOutCommand
FadeOutCommand::FadeOutCommand(int16_t speed)
	: speed(speed)
{
}

CommandResult FadeOutCommand::Execute(ExecutionContext &ctx)
{
	ctx.context.FadeOut(speed);
	return CommandResult::Yield();
}

// FadeInCommand
FadeInCommand::FadeInCommand(int16_t speed)
	: speed(speed)
{
}

CommandResult FadeInCommand::Execute(ExecutionContext &ctx)
{
	ctx.context.FadeIn(speed);
	return CommandResult::Yield();
}

// PlayBGMCommand
PlayBGMCommand::PlayBGMCommand(const std::string &bgmName)
	: bgmName(bgmName)
{
}

CommandResult PlayBGMCommand::Execute(ExecutionContext &ctx)
{
	ctx.context.PlayBGM(bgmName);
	return CommandResult::Continue();
}

// SetFacingCommand
SetFacingCommand::SetFacingCommand(int16_t targetId, Direction dir)
	: targetId(targetId)
	, dir(dir)
{
}

CommandResult SetFacingCommand::Execute(ExecutionContext &ctx)
{
	Actor *actor = ctx.context.GetActorById(targetId);
	if (actor)
	{
		actor->SetDirection(dir);
	}
	return CommandResult::Continue();
}

// SetSpeedCommand
SetSpeedCommand::SetSpeedCommand(int16_t targetId, int16_t speed)
	: targetId(targetId)
	, speed(speed)
{
}

CommandResult SetSpeedCommand::Execute(ExecutionContext &ctx)
{
	Actor *actor = ctx.context.GetActorById(targetId);
	if (actor)
	{
		actor->SetMoveSpeed(speed);
	}
	return CommandResult::Continue();
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
		case CommandType::SET_MOVE_ROUTE:
			return std::make_shared<SetMoveRouteCommand>(packedCmd.val, static_cast<uint16_t>(packedCmd.extraVal), packedCmd.op != 0);
		case CommandType::WAIT:
			return std::make_shared<WaitCommand>(packedCmd.val);
		case CommandType::WAIT_FOR_MOVEMENT:
			return std::make_shared<WaitForMovementCommand>(packedCmd.val);
		case CommandType::FADE_OUT:
			return std::make_shared<FadeOutCommand>(packedCmd.val);
		case CommandType::FADE_IN:
			return std::make_shared<FadeInCommand>(packedCmd.val);
		case CommandType::PLAY_BGM:
			return std::make_shared<PlayBGMCommand>(packedCmd.strParam1);
		case CommandType::SET_FACING:
			return std::make_shared<SetFacingCommand>(packedCmd.val, static_cast<Direction>(packedCmd.op));
		case CommandType::SET_SPEED:
			return std::make_shared<SetSpeedCommand>(packedCmd.val, packedCmd.op);
		default:
			break;
	}
	return nullptr;
}
