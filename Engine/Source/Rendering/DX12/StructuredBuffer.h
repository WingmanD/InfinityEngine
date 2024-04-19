#pragma once

#include "DescriptorHeap.h"
#include "Math/Math.h"
#include "Object.h"
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

template <typename T>
class StructuredBuffer
{
public:
    StructuredBuffer() = default;

    StructuredBuffer(const StructuredBuffer&) = delete;

    StructuredBuffer(StructuredBuffer&& other) noexcept
    {
        Swap(other);
    }

    StructuredBuffer& operator=(const StructuredBuffer&) = delete;

    StructuredBuffer& operator=(StructuredBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }

        return *this;
    }

    ~StructuredBuffer()
    {
        if (_isMapped)
        {
            _uploadHeap->Unmap(0, nullptr);
        }

        if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
        {
            sharedHeap->FreeHeapResourceHandle(_cpuHandle);
        }
    }

    static bool CreateInPlace(StructuredBuffer& buffer, uint32 count, DX12Device& device,
                              std::shared_ptr<DescriptorHeap> srvHeap, Type* type = nullptr)
    {
        buffer._capacity = count;

        if constexpr (IsReflectedType<T>)
        {
            const size_t dataOffset = type->GetDataOffset();
            if (dataOffset == 0)
            {
                LOG(L"Data offset of type {} is 0! Constant buffer needs to copy only data from the object - "
                    "you must set 'DataStart' attribute in your type definition to the first property you want to copy!",
                    Util::ToWString(type->GetName()));
                return false;
            }
            
            buffer._bufferByteSize = static_cast<uint32>(type->GetSize() - dataOffset);
        }
        else
        {
            buffer._bufferByteSize = sizeof(T);
        }

        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(buffer._bufferByteSize * count);
        device.CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&buffer._structuredBuffer));

        const CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        device.CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&buffer._uploadHeap));

        buffer._uploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&buffer._data));

        buffer._cpuHandle = srvHeap->RequestHeapResourceHandle();
        buffer._gpuHandle = srvHeap->GetGPUHeapResourceHandle(buffer._cpuHandle);
        buffer._gpuVirtualAddress = buffer._structuredBuffer->GetGPUVirtualAddress();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = count;
        srvDesc.Buffer.StructureByteStride = buffer._bufferByteSize;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        device.CreateShaderResourceView(buffer._structuredBuffer.Get(), &srvDesc, buffer._cpuHandle);

        return true;
    }

    void Update(DX12GraphicsCommandList* commandList) const
    {
        const CD3DX12_RESOURCE_BARRIER resourceToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1, &resourceToCopyDest);

        commandList->CopyResource(_structuredBuffer.Get(), _uploadHeap.Get());

        const CD3DX12_RESOURCE_BARRIER copyDestToResource = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &copyDestToResource);
    }

    void UpdateSubrange(DX12GraphicsCommandList* commandList, size_t offsetIndex, size_t count) const
    {
        const CD3DX12_RESOURCE_BARRIER resourceToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1, &resourceToCopyDest);

        const size_t alignedOffset = Math::RoundToNearest(static_cast<uint32>(offsetIndex) * _bufferByteSize, 256u);
        const size_t numBytes = (count + offsetIndex - alignedOffset) * _bufferByteSize;

        commandList->CopyBufferRegion(_structuredBuffer.Get(),
                                      alignedOffset,
                                      _uploadHeap.Get(),
                                      alignedOffset,
                                      numBytes);

        const CD3DX12_RESOURCE_BARRIER copyDestToResource = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &copyDestToResource);
    }

    T* GetData() const
    {
        return _data;
    }

    size_t Capacity() const
    {
        return _capacity;
    }

    ID3D12Resource* GetResource() const
    {
        return _structuredBuffer.Get();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const
    {
        return _cpuHandle;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const
    {
        return _gpuHandle;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return _gpuVirtualAddress;
    }

private:
    T* _data = nullptr;
    size_t _capacity = 0;
    uint32 _bufferByteSize = 0;

    bool _isMapped = false;
    ComPtr<ID3D12Resource> _structuredBuffer;
    ComPtr<ID3D12Resource> _uploadHeap;

    std::weak_ptr<DescriptorHeap> _heap;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = 0;

private:
    void Swap(StructuredBuffer& other) noexcept
    {
        _data = other._data;
        other._data = nullptr;

        _capacity = other._capacity;
        other._capacity = 0;

        _bufferByteSize = other._bufferByteSize;
        _isMapped = other._isMapped;
        _structuredBuffer = std::move(other._structuredBuffer);
        _heap = std::move(other._heap);
        _cpuHandle = other._cpuHandle;
        _gpuHandle = other._gpuHandle;
        _gpuVirtualAddress = other._gpuVirtualAddress;
    }
};
