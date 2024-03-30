#include "RenderingSubsystem.h"
#include "Engine/Engine.h"
#include "MaterialParameterTypes.h"

RenderingSubsystem::RenderingSubsystem() : _eventQueue(this)
{
}

RenderingSubsystem& RenderingSubsystem::Get()
{
    return *Engine::Get().GetRenderingSubsystem();
}

bool RenderingSubsystem::Initialize()
{
    EngineSubsystem::Initialize();

    const std::filesystem::path& path = AssetManager::Get().GetProjectRootPath();
    _shaderChangeListener = std::make_unique<ShaderChangeListener>(path / "Engine/Content/Shaders");

    return true;
}

bool RenderingSubsystem::PostInitialize()
{
    EngineSubsystem::PostInitialize();

    _perPassConstants = std::make_unique<PerPassConstants>();
    if (!_perPassConstants->Initialize())
    {
        LOG(L"Failed to initialize global material parameters!");
        DEBUG_BREAK();
        return false;
    }

    return true;
}

EventQueue<RenderingSubsystem>& RenderingSubsystem::GetEventQueue()
{
    return _eventQueue;
}

void RenderingSubsystem::Tick(double deltaTime)
{
    _perPassConstants->Time += deltaTime;
    // todo camera
    // XMStoreFloat4x4(&_perPassConstants->World, XMMatrixTranspose(world));
    // XMStoreFloat4x4(&_perPassConstants->ViewProjection, XMMatrixTranspose(view * proj));
    // _perPassConstants->CameraPosition = pos;
    // _perPassConstants->CameraDirection = target - pos;
    // _perPassConstants->CameraDirection.Normalize();
}

std::shared_ptr<PerPassConstants> RenderingSubsystem::GetPerPassConstants() const
{
    return _perPassConstants;
}
