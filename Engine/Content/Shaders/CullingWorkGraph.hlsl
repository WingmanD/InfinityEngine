#include "ShaderCore.hlsl"
#include "Math.hlsl"

GlobalRootSignature GGlobalRootSignature = {"DescriptorTable(UAV(u0)), CBV(b0)"};

AppendStructuredBuffer<SMInstance> GVisibleInstances : register(u0);
ConstantBuffer<Scene> GScene : register(b0);

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void CoarseCulling(ThreadNodeInputRecord<SMInstance> inputData, [MaxRecords(1)] NodeOutput<SMInstance> OcclusionCulling)
{
    const float3 location = inputData.Get().World[3].xyz;
    const float3 cameraToLocation = location - GScene.CameraLocationWS;
    const float distance = length(cameraToLocation);
    if (distance > 1000.0f)
    {
        return;
    }

    // Frustum culling
    const float3 cameraToLocationDir = normalize(cameraToLocation);

    const float3 xyPlaneNormal = GScene.CameraUp;
    const float3 xyDir = normalize(VectorPlaneProjection(cameraToLocationDir, xyPlaneNormal));

    const float horizontalAngle = AngleFast(xyDir, GScene.CameraForward);
    if (horizontalAngle > GScene.HorizontalFOV)
    {
        return;
    }

    const float3 xzPlaneNormal = cross(GScene.CameraUp, GScene.CameraForward);
    const float3 xzDir = normalize(VectorPlaneProjection(cameraToLocationDir, xzPlaneNormal));
    
    const float verticalAngle = AngleFast(xzDir, GScene.CameraForward);
    if (verticalAngle > GScene.VerticalFOV)
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
