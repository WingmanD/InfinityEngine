#pragma once

#include "DescriptorHeap.h"
#include "DX12Statics.h"
#include "Object.h"
#include "Rendering/DX12/DX12RenderingCore.h"
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

template <typename T>
class ConstantBuffer
{
public:
    T* Data;

public:
    ConstantBuffer() = default;

    ConstantBuffer(const ConstantBuffer&) = delete;

    ConstantBuffer(ConstantBuffer&& other) noexcept
    {
        Swap(other);

        other.Data = nullptr;
        other._mappedData = nullptr;
    }

    ConstantBuffer& operator=(const ConstantBuffer&) = delete;

    ConstantBuffer& operator=(ConstantBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Swap();

            other.Data = nullptr;
            other._mappedData = nullptr;
        }

        return *this;
    }

    ~ConstantBuffer()
    {
        if (_isMapped)
        {
            _buffer->Unmap(0, nullptr);
        }

        // todo assets that own constant buffers are destroyed when asset manager is destroyed -
        // after rendering subsystem is destroyed which destroys descriptor heaps, causing DX12 warnings about live objects
        if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
        {
            sharedHeap->FreeHeapResourceHandle(_cpuHandle);
        }
    }

    static bool CreateInPlace(ConstantBuffer& buffer, T* data, uint64 count, DX12Device* device,
                              const std::shared_ptr<DescriptorHeap>& heap)
    {
        if (data == nullptr)
        {
            DEBUG_BREAK();
            return false;
        }

        if (device == nullptr || heap == nullptr)
        {
            return false;
        }

        if constexpr (IsReflectedType<T>)
        {
            const Object* object = dynamic_cast<Object*>(data);

            const size_t dataOffset = object->GetType()->GetDataOffset();
            if (dataOffset == 0)
            {
                LOG(L"Data offset of type {} is 0! Constant buffer needs to copy only data from the object - "
                    "you must set 'DataStart' attribute in your type definition to the first property you want to copy!",
                    Util::ToWString(object->GetType()->GetName()));
                return false;
            }

            if (count != 1)
            {
                LOG(L"Constant buffer can only copy data from a single object!");
                DEBUG_BREAK();

                return false;
            }

            buffer._size = (object->GetType()->GetSize() - dataOffset);
            buffer._offset = dataOffset;
        }
        else
        {
            buffer._size = sizeof(T) * count;
            buffer._offset = 0;
        }

        buffer._heap = heap;
        buffer.Data = data;

        const uint64 size = DX12Statics::CalculateConstantBufferSize(buffer._size);

        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

        ThrowIfFailed(device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&buffer._buffer)));

        ThrowIfFailed(buffer._buffer->Map(0, nullptr, reinterpret_cast<void**>(&buffer._mappedData)));
        buffer._isMapped = true;

        buffer._cpuHandle = heap->RequestHeapResourceHandle();
        buffer._gpuHandle = heap->GetGPUHeapResourceHandle(buffer._cpuHandle);
        buffer._gpuVirtualAddress = buffer._buffer->GetGPUVirtualAddress();

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = buffer._gpuVirtualAddress;
        cbvDesc.SizeInBytes = static_cast<uint32>(size);
        device->CreateConstantBufferView(&cbvDesc, buffer._cpuHandle);

        return true;
    }

    void Update() const
    {
        UpdateSubrange(0, _size);
    }

    void UpdateSubrange(size_t offset, size_t size) const
    {
        const std::byte* byteData = reinterpret_cast<std::byte*>(Data) + _offset + offset;
        memcpy(_mappedData + offset, byteData, size);
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

    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const
    {
        return _gpuVirtualAddress;
    }

private:
    size_t _size = 0;
    size_t _offset = 0;

    bool _isMapped = false;
    ComPtr<ID3D12Resource> _buffer;
    ComPtr<ID3D12Resource> _uploadHeap;
    std::weak_ptr<DescriptorHeap> _heap;

    std::byte* _mappedData = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = 0;

private:
    void Swap(ConstantBuffer& other) noexcept
    {
        Data = other.Data;
        _size = other._size;
        _offset = other._offset;
        _isMapped = other._isMapped;
        _buffer = std::move(other._buffer);
        _heap = std::move(other._heap);
        _mappedData = other._mappedData;
        _cpuHandle = other._cpuHandle;
        _gpuHandle = other._gpuHandle;
        _gpuVirtualAddress = other._gpuVirtualAddress;

        other.Data = nullptr;
        other._mappedData = nullptr;
    }
};
