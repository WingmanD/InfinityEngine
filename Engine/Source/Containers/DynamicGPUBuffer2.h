#pragma once

#include "ReflectionShared.h"
#include "ManagedBuffer.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include <memory>

class GPUBuffer;

template <typename T> requires !IsReflectedType<T>
class DynamicGPUBuffer2 : public ManagedBuffer<T>
{
public:
    bool Initialize()
    {
        _buffer = RenderingSubsystem::Get().CreateBuffer(GPUBuffer::EType::Default);

        return _buffer->Initialize(
            ManagedBuffer<T>::GetData(),
            static_cast<uint32>(ManagedBuffer<T>::Capacity()),
            sizeof(T)
        );
    }
    
    GPUBuffer& GetBuffer() const
    {
        return *_buffer.get();
    }

    template <typename U>
    U& GetBuffer() const
    {
        return static_cast<U&>(GetBuffer());
    }

protected:
    virtual void OnReallocated(size_t capacity) override
    {
        ManagedBuffer<T>::OnReallocated(capacity);
        
        if (!_buffer->Initialize(
            ManagedBuffer<T>::GetData(),
            static_cast<uint32>(ManagedBuffer<T>::Capacity()),
            sizeof(T)
        ))
        {
            DEBUG_BREAK();
            LOG(L"Could not reallocate dynamic GPU buffer!");
        }

        ManagedBuffer<T>::InvalidateWholeBuffer();
    }
    
private:
    std::unique_ptr<GPUBuffer> _buffer;
};
