#include "RenderingSubsystem.h"
#include "Engine/Engine.h"
#include "MaterialParameterTypes.h"
#include "Rendering/StaticMesh.h"

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

    _scene = std::make_unique<Scene>();
    if (!_scene->Initialize())
    {
        LOG(L"Failed to initialize global material parameters!");
        DEBUG_BREAK();
        return false;
    }

    StaticMesh::GetMeshInfoBuffer().Initialize();

    return true;
}

EventQueue<RenderingSubsystem>& RenderingSubsystem::GetEventQueue()
{
    return _eventQueue;
}

void RenderingSubsystem::Tick(double deltaTime)
{
    _scene->Time += static_cast<float>(deltaTime);
}

std::shared_ptr<Scene> RenderingSubsystem::GetScene() const
{
    return _scene;
}
