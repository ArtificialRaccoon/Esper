#pragma once
#include <string>

struct TileMapLayer
{
    char name[9]; 
    uint16_t width;
    uint16_t height;
    std::vector<uint16_t> tileData;
};