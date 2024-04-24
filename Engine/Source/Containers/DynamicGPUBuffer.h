#pragma once

#include "BorrowedManagedBuffer.h"
#include "Rendering/IDynamicGPUBufferProxy.h"

class DynamicGpuBufferBase
{
};

template <typename T>
class DynamicGPUBuffer : public DynamicGpuBufferBase, public BorrowedManagedBuffer<T>
{
public:
    explicit DynamicGPUBuffer(T* data = nullptr,
                     uint32 capacity = 0,
                     uint32 count = 0) :
        BorrowedManagedBuffer<T>(data, capacity, count)
    {
    }

    void Reserve(uint32 capacity)
    {
        _proxy->Reserve(capacity);
    }

    void SetProxy(std::unique_ptr<IDynamicGPUBufferProxy>&& proxy)
    {
        _proxy = std::move(proxy);

        _proxy->Initialize(*this);
    }

    IDynamicGPUBufferProxy* GetProxy() const
    {
        return _proxy.get();
    }

    template <typename U>
    U* GetProxy() const
    {
        return dynamic_cast<U*>(_proxy.get());
    }

private:
    std::unique_ptr<IDynamicGPUBufferProxy> _proxy = nullptr;
};
