#pragma once

#include "CoreMinimal.h"

class GPUBuffer
{
public:
    enum class EType : uint8
    {
        Default,
        UnorderedAccess,
        ReadBack,
        Counted
    };

public:
    explicit GPUBuffer() = default;
    
    GPUBuffer(const GPUBuffer& other) = delete;
    GPUBuffer(GPUBuffer&& other) noexcept;
    GPUBuffer& operator=(const GPUBuffer& other) = delete;
    GPUBuffer& operator=(GPUBuffer&& other) noexcept;
    
    virtual ~GPUBuffer() = default;

    bool Initialize(void* data, uint32 capacity, uint32 elementSize);

    void* GetData() const;
    uint32 GetCapacity() const;
    uint32 GetElementSize() const;

protected:
    void* Data = nullptr;

protected:
    virtual bool InitializeImplementation(uint32 capacity, uint32 elementSize) = 0;

private:
    uint32 _capacity = 0;
    uint32 _elementSize = 0;
};
