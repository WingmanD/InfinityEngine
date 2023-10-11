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

    ConstantBuffer(ID3D12Device* device, DescriptorHeap* heap) : _heap(heap)
    {
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

        _cpuHandle = _heap->RequestHeapResourceHandle(size);
        _gpuHandle = _heap->GetGPUHeapResourceHandle(_cpuHandle);

        // // todo everything must be 256 byte aligned on the heap - DynamicDescriptorHeap must handle this
        // _cpuHandle = _heap->GetHeap()->GetCPUDescriptorHandleForHeapStart();
        // _gpuHandle = _heap->GetHeap()->GetGPUDescriptorHandleForHeapStart();

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = _buffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = size;
        device->CreateConstantBufferView(&cbvDesc, _cpuHandle);
    }

    ~ConstantBuffer()
    {
        // todo buffer may not be mapped at all times
        // todo weak ptr to heap - rendering subsystem must have a shared ptr to heap unfortunately
        _buffer->Unmap(0, nullptr);
        _heap->FreeHeapResourceHandle(_cpuHandle);
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
    ComPtr<ID3D12Resource> _buffer;
    DescriptorHeap* _heap;

    std::byte* _mappedData = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
};
