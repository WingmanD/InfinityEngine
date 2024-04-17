﻿#pragma once

#include "StructuredBuffer.h"
#include "Rendering/IDynamicGPUBufferProxy.h"
#include "Containers/DynamicGPUBuffer.h"
#include "Rendering/DX12/DX12RenderingSubsystem.h"

struct SMInstance;
class StaticMeshInstance;
class DescriptorHeap;
class InstanceBuffer;

template <typename T>
class DynamicGPUBufferUploader : public IDynamicGPUBufferProxy
{
public:
    DynamicGPUBufferUploader() = default;

    DynamicGPUBufferUploader(const DynamicGPUBufferUploader&) = delete;

    DynamicGPUBufferUploader(DynamicGPUBufferUploader&& other) noexcept
    {
        Swap(other);
    }

    DynamicGPUBufferUploader& operator=(const DynamicGPUBufferUploader&) = delete;

    DynamicGPUBufferUploader& operator=(DynamicGPUBufferUploader&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }

        return *this;
    }

    virtual ~DynamicGPUBufferUploader() override = default;

    void Update(DX12GraphicsCommandList* commandList)
    {
        const DArray<uint32>& dirtyIndices = _owningBuffer->GetDirtyIndices();
        if (dirtyIndices.Count() > 16) // todo optimize, find out the best number  
        {
            _structuredBuffer.Update(commandList);
        }
        else
        {
            for (const size_t dirtyIndex : dirtyIndices)
            {
                _structuredBuffer.UpdateSubrange(commandList, dirtyIndex, 1);
            }
        }

        _owningBuffer->ClearDirtyIndices();
    }

    DynamicGPUBuffer<T>& GetOwningBuffer()
    {
        return *_owningBuffer;
    }

    StructuredBuffer<SMInstance>& GetStructuredBuffer()
    {
        return _structuredBuffer;
    }

    // IDynamicGPUBufferProxy
public:
    virtual bool Initialize(DynamicGpuBufferBase& buffer) override
    {
        _owningBuffer = &static_cast<DynamicGPUBuffer<T>&>(buffer);

        constexpr uint32 initialCapacity = 16;

        DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
        if (!StructuredBuffer<SMInstance>::CreateInPlace(_structuredBuffer,
                                                         initialCapacity,
                                                         *renderingSubsystem.GetDevice(),
                                                         renderingSubsystem.GetCBVHeap()))
        {
            DEBUG_BREAK();
            return false;
        }

        _owningBuffer->Initialize(_structuredBuffer.GetData(), initialCapacity, 0ull);

        return true;
    }

    virtual void Reserve(uint32 capacity) override
    {
        if (_structuredBuffer.Capacity() >= capacity)
        {
            return;
        }

        const uint32 newCapacity = Math::NextPowerOfTwo(capacity);

        DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

        StructuredBuffer<SMInstance> newStructuredBuffer;

        if (!StructuredBuffer<SMInstance>::CreateInPlace(
            newStructuredBuffer,
            newCapacity,
            *renderingSubsystem.GetDevice(),
            renderingSubsystem.GetCBVHeap()))
        {
            DEBUG_BREAK();
            return;
        }

        const uint32 count = static_cast<uint32>(_owningBuffer->Count());
        memcpy(newStructuredBuffer.GetData(), _structuredBuffer.GetData(), count * sizeof(T));

        _owningBuffer->Initialize(newStructuredBuffer.GetData(), newCapacity, count);
        _structuredBuffer = std::move(newStructuredBuffer);
    }

private:
    DynamicGPUBuffer<T>* _owningBuffer;

    StructuredBuffer<SMInstance> _structuredBuffer;

private:
    void Swap(DynamicGPUBufferUploader& other) noexcept
    {
        _owningBuffer = other._owningBuffer;
        other._owningBuffer = nullptr;

        _structuredBuffer = std::move(other._structuredBuffer);
    }
};
