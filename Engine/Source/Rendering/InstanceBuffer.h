#pragma once

#include "Containers/DynamicGPUBuffer.h"
#include "Math/MathFwd.h"

struct SMInstance
{
    Matrix World;
    uint32 MeshID;
    uint32 MaterialID;
    uint32 MaterialIndex;
};

class InstanceBuffer : public DynamicGPUBuffer<SMInstance>
{
public:
    InstanceBuffer() = default;
};
