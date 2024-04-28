#include "ShaderCore.hlsl"

#define BITONIC_BLOCK_SIZE 128
#define TRANSPOSE_BLOCK_SIZE 16

struct SMBitonicSortConstants
{
    uint Level;
    uint LevelMask;
    uint Width;
    uint Height;
};

ConstantBuffer<SMBitonicSortConstants> GBitonicSortConstants : register (b0);
RWStructuredBuffer<SMInstance> GOutputInstances : register (u0);

groupshared SMInstance GSharedData[BITONIC_BLOCK_SIZE];

[numthreads(BITONIC_BLOCK_SIZE, 1, 1)]
void CS(uint3 groupID: SV_GroupID,
        uint3 threadID: SV_DispatchThreadID,
        uint3 groupThreadID: SV_GroupThreadID,
        uint groupIndex: SV_GroupIndex)
{
    GSharedData[groupIndex] = GOutputInstances[threadID.x];
    GroupMemoryBarrierWithGroupSync();

    for (unsigned int j = GBitonicSortConstants.Level >> 1; j > 0; j >>= 1)
    {
        SMInstance result;

        uint meshIDA = GSharedData[groupIndex & ~j].MeshID;
        uint meshIDB = GSharedData[groupIndex | j].MeshID;
        if (meshIDA == 0) meshIDA = -1;
        if (meshIDB == 0) meshIDB = -1;

        const bool rhs = (bool)(GBitonicSortConstants.LevelMask & threadID.x);
        const bool byMesh = (meshIDA < meshIDB) == rhs;
        const bool byMaterial = (GSharedData[groupIndex & ~j].MaterialID <= GSharedData[groupIndex | j].MaterialID) ==
            rhs;
        const bool byLOD = (GSharedData[groupIndex & ~j].LOD <= GSharedData[groupIndex | j].LOD) == rhs;

        if (meshIDA == -1 || meshIDB == -1)
        {
            if (byMesh)
            {
                result = GSharedData[groupIndex ^ j];
            }
            else
            {
                result = GSharedData[groupIndex];
            }
        }
        else 
        {
            if (byMaterial || byMesh || byLOD)
            {
                result = GSharedData[groupIndex ^ j];
            }
            else
            {
                result = GSharedData[groupIndex];
            }
        }

        GroupMemoryBarrierWithGroupSync();
        GSharedData[groupIndex] = result;
        GroupMemoryBarrierWithGroupSync();
    }

    GOutputInstances[threadID.x] = GSharedData[groupIndex];
}

StructuredBuffer<SMInstance> GInputInstances : register (t0);

groupshared SMInstance GTransposeSharedData[TRANSPOSE_BLOCK_SIZE * TRANSPOSE_BLOCK_SIZE];

[numthreads(TRANSPOSE_BLOCK_SIZE, TRANSPOSE_BLOCK_SIZE, 1)]
void MatrixTranspose(uint3 groupID: SV_GroupID,
                     uint3 threadID: SV_DispatchThreadID,
                     uint3 groupThreadID: SV_GroupThreadID,
                     uint groupIndex: SV_GroupIndex)
{
    GTransposeSharedData[groupIndex] = GInputInstances[threadID.y * GBitonicSortConstants.Width + threadID.x];

    GroupMemoryBarrierWithGroupSync();

    const uint2 xy = threadID.yx - groupThreadID.yx + groupThreadID.xy;
    GOutputInstances[xy.y * GBitonicSortConstants.Height + xy.x] =
        GTransposeSharedData[groupThreadID.x * TRANSPOSE_BLOCK_SIZE + groupThreadID.y];
}
