#pragma once

#include "CoreMinimal.h"
#include "Rendering/DX12/DX12RenderingCore.h"
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class DescriptorHeap
{
public:
    DescriptorHeap() = default;
    DescriptorHeap(DX12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description);
    virtual ~DescriptorHeap() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE RequestHeapResourceHandle();
    void FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const;

    ComPtr<ID3D12DescriptorHeap>& GetHeap();

private:
    ComPtr<ID3D12DescriptorHeap> _heap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE _heapTop = {};

    uint32 _descriptorSize = 0;
    std::vector<int32> _availableHeapResourceOffsets;

private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE& GetHeapTop();

    size_t CalculateOffset(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const;
};
