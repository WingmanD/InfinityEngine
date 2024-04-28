#include "ShaderCore.hlsl"
#include "Math.hlsl"

GlobalRootSignature GGlobalRootSignature = {"DescriptorTable(UAV(u0)), CBV(b0), SRV(t0)"};

AppendStructuredBuffer<SMInstance> GVisibleInstances : register(u0);
ConstantBuffer<Scene> GScene : register(b0);
StructuredBuffer<MeshInfo> GMeshInfo : register(t0);

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void CoarseCulling(ThreadNodeInputRecord<SMInstance> inputData, [MaxRecords(1)] NodeOutput<SMInstance> OcclusionCulling)
{
    // Distance culling and culling of objects behind the camera
    const float distance = DistanceFromPlane(
        inputData.Get().World[3].xyz,
        GScene.CameraForward,
        GScene.CameraLocationWS
    );
    if (distance < 0.0f || distance > GScene.DrawDistance)
    {
        return;
    }
    
    // Frustum culling
    uint meshID = inputData.Get().MeshID;
    
    const float4 min = float4(GMeshInfo[meshID].BoundingBox.Min, 1.0f);
    const float4 max = float4(GMeshInfo[meshID].BoundingBox.Max, 1.0f);
    
    const float4 vertices[8] = {
        min,
        float4(min.x, min.y, max.z, 1.0f),
        float4(min.x, max.y, min.z, 1.0f),
        float4(min.x, max.y, max.z, 1.0f),
        float4(max.x, min.y, min.z, 1.0f),
        float4(max.x, min.y, max.z, 1.0f),
        float4(max.x, max.y, min.z, 1.0f),
        max
    };
    
    const float4x4 MVP = mul(inputData.Get().World, GScene.ViewProjection);
    
    bool anyVertexInsideNDC = false;
    float4 vertexCS;
    
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        vertexCS = mul(vertices[i], MVP);
    
        anyVertexInsideNDC |=
            all(vertexCS.xy >= -vertexCS.w) &&
            all(vertexCS.xy <= vertexCS.w) &&
            vertexCS.z >= 0.0f && vertexCS.z <= vertexCS.w;
    }
    
    if (!anyVertexInsideNDC)
    {
        return;
    }

    ThreadNodeOutputRecords<SMInstance> output = OcclusionCulling.GetThreadNodeOutputRecords(1);
    output.Get() = inputData.Get();
    output.OutputComplete();
}

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void OcclusionCulling(ThreadNodeInputRecord<SMInstance> inputData)
{
    GVisibleInstances.Append(inputData.Get());
}
