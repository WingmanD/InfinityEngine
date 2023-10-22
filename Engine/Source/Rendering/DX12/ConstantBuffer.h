#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "d3dx12.h"
#include "DescriptorHeap.h"
#include "DX12Statics.h"

using Microsoft::WRL::ComPtr;

template <typename T>
class ConstantBuffer
{
public:
    T Data;

public:
    ConstantBuffer() = default;

    ConstantBuffer(ID3D12Device* device, const std::shared_ptr<DescriptorHeap>& heap)
    {
        if (device == nullptr || heap == nullptr)
        {
            DEBUG_BREAK();
            return;
        }
        
        _heap = heap;
        
        constexpr uint32 size = DX12Statics::CalculateConstantBufferSize(sizeof(T));

        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

        ThrowIfFailed(device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_buffer)));

        ThrowIfFailed(_buffer->Map(0, nullptr, reinterpret_cast<void**>(&_mappedData)));
        _isMapped = true;

        _cpuHandle = heap->RequestHeapResourceHandle();
        _gpuHandle = heap->GetGPUHeapResourceHandle(_cpuHandle);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = _buffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = size;
        device->CreateConstantBufferView(&cbvDesc, _cpuHandle);
    }

    ~ConstantBuffer()
    {
        if (_isMapped)
        {
            _buffer->Unmap(0, nullptr);
        }

        if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
        {
            sharedHeap->FreeHeapResourceHandle(_cpuHandle);
        }
    }

    void Update() const
    {
        memcpy(_mappedData, &Data, sizeof(T));
    }

    ID3D12Resource* GetResource() const
    {
        return _buffer.Get();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const
    {
        return _cpuHandle;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const
    {
        return _gpuHandle;
    }

private:
    bool _isMapped = false;
    ComPtr<ID3D12Resource> _buffer;
    std::weak_ptr<DescriptorHeap> _heap;

    std::byte* _mappedData = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
};
