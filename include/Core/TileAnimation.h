#pragma once
#include <vector>

struct __attribute__((packed)) TileFrame
{
	uint16_t tileId;
	uint16_t duration;
};

struct TileAnimation
{
	uint16_t tileId;
	uint16_t frameCount;
	std::vector<TileFrame> frames;

	int currentFrameIndex = 0;
	int framesRemaining = 0;
};