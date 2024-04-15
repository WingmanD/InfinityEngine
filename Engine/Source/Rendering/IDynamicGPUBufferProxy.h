#pragma once

class DynamicGpuBufferBase;

class IDynamicGPUBufferProxy
{
public:
    virtual bool Initialize(DynamicGpuBufferBase& buffer) = 0;
    virtual void Reserve(uint32 capacity) = 0;
    
    virtual ~IDynamicGPUBufferProxy() = default;
};
