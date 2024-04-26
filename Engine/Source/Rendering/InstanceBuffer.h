#pragma once

#include "Containers/DynamicGPUBuffer.h"
#include "Math/MathFwd.h"

struct SMInstance
{
    Matrix World;
    uint32 MeshID;
    uint32 LOD;
    uint32 MaterialID;
    uint32 MaterialIndex;
    Vector3 AABBMin;
    float padding;
    Vector3 AABBMax;
    float padding2;
};

class InstanceBuffer : public DynamicGPUBuffer<SMInstance>
{
public:
    InstanceBuffer() = default;
};
