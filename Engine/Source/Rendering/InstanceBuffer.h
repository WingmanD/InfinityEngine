#pragma once

#include "Containers/DynamicGPUBuffer.h"
#include "Math/MathFwd.h"

struct alignas(16) SMInstance
{
    Matrix World;
    uint32 MeshID;
    uint32 LOD;
    uint32 MaterialID;
    uint32 MaterialIndex;
    uint32 Count;
};

class InstanceBuffer : public DynamicGPUBuffer<SMInstance>
{
public:
    InstanceBuffer() = default;
};
