﻿#define FORWARD_PLUS_GROUP_DIM 16
#define FORWARD_PLUS_GROUP_THREADS (FORWARD_PLUS_GROUP_DIM * FORWARD_PLUS_GROUP_DIM)
#define FORWARD_PLUS_TILE_LIGHTS 256

struct Tile
{
    uint StartIndex;
    uint LightCount;
};