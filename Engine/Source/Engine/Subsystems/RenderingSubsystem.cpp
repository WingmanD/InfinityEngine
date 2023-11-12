#include "RenderingSubsystem.h"
#include "Engine/Engine.h"

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

EventQueue<RenderingSubsystem>& RenderingSubsystem::GetEventQueue()
{
    return _eventQueue;
}
