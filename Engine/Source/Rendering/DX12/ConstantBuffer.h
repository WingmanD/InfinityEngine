#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "DescriptorHeap.h"
#include "DX12Statics.h"
#include "Object.h"
using Microsoft::WRL::ComPtr;

// todo we need another constant buffer that is not placed on the heap
class ConstantBuffer
{
public:
    Object* Data;

public:
    ConstantBuffer() = default;
    ~ConstantBuffer();

    static bool CreateInPlace(ConstantBuffer& buffer, Object* parameter, ID3D12Device* device, const std::shared_ptr<DescriptorHeap>& heap);
    
    void Update() const;

    ID3D12Resource* GetResource() const;

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const;

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const;

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;
    
private:
    size_t _size = 0;
    size_t _offset = 0;
    
    bool _isMapped = false;
    ComPtr<ID3D12Resource> _buffer;
    std::weak_ptr<DescriptorHeap> _heap;

    std::byte* _mappedData = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = 0;
};
