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
    
    const float4 bbMin = float4(GMeshInfo[meshID].BoundingBox.Min, 1.0f);
    const float4 bbMax = float4(GMeshInfo[meshID].BoundingBox.Max, 1.0f);
    
    const float4 vertices[8] = {
        bbMin,
        float4(bbMin.x, bbMin.y, bbMax.z, 1.0f),
        float4(bbMin.x, bbMax.y, bbMin.z, 1.0f),
        float4(bbMin.x, bbMax.y, bbMax.z, 1.0f),
        float4(bbMax.x, bbMin.y, bbMin.z, 1.0f),
        float4(bbMax.x, bbMin.y, bbMax.z, 1.0f),
        float4(bbMax.x, bbMax.y, bbMin.z, 1.0f),
        bbMax
    };
    
    const float4x4 MVP = mul(inputData.Get().World, GScene.ViewProjection);
    
    bool anyVertexInsideNDC = false;
    float4 vertexCS;

    float4 transformedMin = float4(-1e6, -1e6, -1e6, 1.0f);
    float4 transformedMax = float4(1e6, 1e6, 1e6, 1.0f);
    
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        vertexCS = mul(vertices[i], MVP);
    
        // anyVertexInsideNDC |=
        //     all(vertexCS.xy >= -vertexCS.w) &&
        //     all(vertexCS.xy <= vertexCS.w) &&
        //     vertexCS.z >= 0.0f && vertexCS.z <= vertexCS.w;
        //
        // if (anyVertexInsideNDC)
        // {
        //     break;
        // }
        
        transformedMin = min(transformedMin, vertexCS);
        transformedMax = max(transformedMax, vertexCS);
    }

    //if (!anyVertexInsideNDC)
    {
        const float4 ndcMin = float4(-1.0f, -1.0f, 0.0f, 1.0f);
        const float4 ndcMax = float4(1.0f, 1.0f, 1.0f, 1.0f);

        if (!(ndcMin.x <= transformedMax.x && ndcMax.x >= transformedMin.x &&
              ndcMin.y <= transformedMax.y && ndcMax.y >= transformedMin.y &&
              ndcMin.z <= transformedMax.z && ndcMax.z >= transformedMin.z))
        {
            return;
        }
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
