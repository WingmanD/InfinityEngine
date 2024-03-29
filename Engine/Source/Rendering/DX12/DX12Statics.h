﻿#pragma once

#include "Core.h"

namespace DX12Statics
{
    constexpr uint64 CalculateConstantBufferSize(uint64 byteSize)
    {
        return (byteSize + 255) & ~255;
    }
}
