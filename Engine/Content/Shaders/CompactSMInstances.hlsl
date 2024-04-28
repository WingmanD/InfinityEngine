#include "ShaderCore.hlsl"

#define NUM_THREADS 32

struct ElementCounter
{
    uint Count;
    uint Capacity;
};

ConstantBuffer<ElementCounter> GNumElements : register(b0);
RWStructuredBuffer<SMInstance> GSortedInstances : register(u0);

[numthreads(NUM_THREADS, 1, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    const uint blockSize = GNumElements.Capacity / NUM_THREADS;
    
    uint currentInstanceStartIndex = threadID.x * blockSize;
    GSortedInstances[currentInstanceStartIndex].Count = 1;
    
    const uint total = min(currentInstanceStartIndex + blockSize, GNumElements.Count);
    for (uint i = currentInstanceStartIndex + 1; i < total; ++i)
    {
        if (GSortedInstances[currentInstanceStartIndex].MaterialID == GSortedInstances[i].MaterialID &&
            GSortedInstances[currentInstanceStartIndex].MeshID == GSortedInstances[i].MeshID &&
            GSortedInstances[currentInstanceStartIndex].LOD == GSortedInstances[i].LOD)
        {
            ++GSortedInstances[currentInstanceStartIndex].Count;
        }
        else
        {
            currentInstanceStartIndex = i;
            GSortedInstances[currentInstanceStartIndex].Count = 1;
        }
    }
}
