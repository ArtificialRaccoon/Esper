#pragma once
#include <cstdint>
#include "Core/GlobalEnumerations.h"

enum class EventTriggerType : uint8_t
{
	ACTION_BUTTON,
	PLAYER_TOUCH,
	EVENT_TOUCH,
	AUTORUN,
	PARALLEL,
	NONE
};

enum class CommandType : uint8_t
{
	NONE = 0,
	SHOW_TEXT = 1,
	CONTROL_SELF_SWITCH = 2,
	CONTROL_SELF_VAR = 3,
	CONTROL_VAR = 4,
	PLAY_SFX = 5,
	TRANSFER_PLAYER = 6,
	SET_MOVE_ROUTE = 7,
	WAIT = 8,
	WAIT_FOR_MOVEMENT = 9,
	FADE_OUT = 10,
	FADE_IN = 11,
	PLAY_BGM = 12,
	SET_FACING = 13,
	SET_SPEED = 14,
	SHOW_CHOICES = 15
};

struct __attribute__((packed)) EventCommand
{
	CommandType type = CommandType::NONE;
	uint8_t op = 0; 			// 0=SET, 1=ADD, or boolean true/false
	int16_t val = 0; 			// string index or var value or tileX
	int16_t extraVal = 0; 		// tileY
	char strParam1[24] = { 0 }; // varName, selfSwitch, sfxName, mapName
	char padding[2] = { 0 };
};

struct __attribute__((packed)) EventPageHeader
{
	EventTriggerType trigger = EventTriggerType::NONE;
	uint8_t spriteFrame = 0;
	uint8_t isWalkable = 1;
	int16_t variableThreshold = 0;
	EventMoveType moveType = EventMoveType::FIXED;
	uint8_t moveSpeed = 1;
	uint8_t moveFrequency = 30;
	char movePath[16] = { 0 };
	char switchCondition[24] = { 0 };
	char variableCondition[24] = { 0 };
	char selfVariableCondition[24] = { 0 };
	char selfSwitchCondition[8] = { 0 };
	char spriteName[8] = { 0 };
	uint16_t commandCount = 0;
};

enum class EventClassType : uint8_t
{
	TRIGGER = 0,
	ACTOR = 1
};

struct __attribute__((packed)) GameEvent
{
	uint16_t eventId;
	uint8_t eventType = static_cast<uint8_t>(EventClassType::TRIGGER);
	int16_t tileX;
	int16_t tileY;
	int16_t endTileX;
	int16_t endTileY;
	uint16_t pageCount;
};
