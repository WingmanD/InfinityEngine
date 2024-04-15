#pragma once

#include "UtilMacros.h"
#include "Containers/DynamicGPUBuffer.h"
#include "Math/MathFwd.h"

struct SMInstance
{
    Matrix World;
    uint64 MeshID;
    PADDING()
    PADDING()
    uint64 MaterialID;
    PADDING()
    PADDING()
    uint32 MaterialIndex;
};

class InstanceBuffer : public DynamicGPUBuffer<SMInstance>
{
public:
    InstanceBuffer() = default;
};
