#pragma once

#include "CoreMinimal.h"

namespace DX12Statics
{
    constexpr uint64 CalculateConstantBufferSize(uint64 byteSize)
    {
        return (byteSize + 255) & ~255;
    }

    void Transition(DX12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
    void TransitionUAV(DX12GraphicsCommandList& commandList, ID3D12Resource* resource);
}
