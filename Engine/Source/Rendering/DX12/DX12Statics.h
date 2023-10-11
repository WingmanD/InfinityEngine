#pragma once

#include "Core.h"

namespace DX12Statics
{
    constexpr uint32 CalculateConstantBufferSize(uint32 byteSize)
    {
        return (byteSize + 255) & ~255;
    }
}
