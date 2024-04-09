#pragma once

#include "CoreMinimal.h"
#include "ConstantBuffer.h"
#include "Containers/DArray.h"
#include <memory>

class DescriptorHeap;

template <typename T>
class DynamicUploadBuffer
{
public:
    DynamicUploadBuffer() = default;

    DynamicUploadBuffer(const DynamicUploadBuffer&) = delete;
    DynamicUploadBuffer(DynamicUploadBuffer&& other) noexcept = delete;

    DynamicUploadBuffer& operator=(const DynamicUploadBuffer&) = delete;
    DynamicUploadBuffer& operator=(DynamicUploadBuffer&& other) noexcept = delete;

    ~DynamicUploadBuffer() = default;

    bool Initialize(DX12Device* device, size_t capacity, const std::shared_ptr<DescriptorHeap>& heap)
    {
        _device = device;
        _heap = heap;
        _bufferCapacity = capacity;

        _data.Reserve(capacity);

        return ConstantBuffer<T>::CreateInPlace(_buffer, _data.GetData(), _bufferCapacity, device, heap);
    }

    size_t Add(const T& data)
    {
        _data.Add(data);

        if (_data.Count() >= _bufferCapacity)
        {
            if (!ConstantBuffer<T>::CreateInPlace(_buffer, _data.GetData(), _bufferCapacity, _device, _heap))
            {
                DEBUG_BREAK();
                return INDEX_NONE;
            }

            _bufferCapacity = _data.Capacity();
        }

        const size_t index = _data.Count() - 1;
        _buffer.UpdateSubrange(index, 1);

        return index;
    }

    T& operator[](size_t index)
    {
        return _data[index];
    }

    void Update(size_t index)
    {
        _buffer.UpdateSubrange(index, 1);
    }

    void RemoveAtSwap(size_t index)
    {
        _data.RemoveAtSwap(index);
        _buffer.UpdateSubrange(index, 1);
    }

    void Clear()
    {
        _data.Clear();
        _buffer.Update();
    }

private:
    DX12Device* _device = nullptr;
    std::weak_ptr<DescriptorHeap> _heap;

    DArray<T> _data;

    ConstantBuffer<T> _buffer;
    size_t _bufferCapacity = 0;
};
