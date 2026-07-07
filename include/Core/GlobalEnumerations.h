#pragma once

enum class Direction
{
	DOWN,
	RIGHT,
	UP,
	LEFT
};

enum class EventMoveType : uint8_t
{
	FIXED = 0,
	RANDOM_WANDER = 1,
	PATH_FOLLOW = 2
};