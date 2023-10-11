#pragma once

#include "Core.h"
#include <d3d12.h>
#include <wrl/client.h>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class DescriptorHeap
{
public:
    DescriptorHeap() = default;
    DescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description);
    virtual ~DescriptorHeap() = default;

    virtual D3D12_CPU_DESCRIPTOR_HANDLE RequestHeapResourceHandle(uint32 size = 0u) = 0;
    virtual void FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint32 size = 0u) = 0;

    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const;

    ComPtr<ID3D12DescriptorHeap>& GetHeap();
    
protected:
    CD3DX12_CPU_DESCRIPTOR_HANDLE& GetHeapTop();

    int32 CalculateOffset(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const;
    
private:
    ComPtr<ID3D12DescriptorHeap> _heap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE _heapTop = {};
};
