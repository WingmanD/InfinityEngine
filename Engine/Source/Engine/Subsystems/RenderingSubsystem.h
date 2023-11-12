#pragma once

#include "Core.h"
#include "EventQueue.h"
#include "Engine/Subsystems/EngineSubsystem.h"
#include "Filesystem/ShaderChangeListener.h"
#include <memory>


class Window;
class StaticMeshRenderingData;
class MaterialRenderingData;
class MaterialParameterMap;
class Texture;
class RenderTarget;

class RenderingSubsystem : public EngineSubsystem
{
public:
    RenderingSubsystem();

    static RenderingSubsystem& Get();

    virtual bool Initialize() override;
    
    virtual std::shared_ptr<Window> ConstructWindow(const std::wstring& title) = 0;

    virtual std::unique_ptr<StaticMeshRenderingData> CreateStaticMeshRenderingData() = 0;
    virtual std::unique_ptr<MaterialRenderingData> CreateMaterialRenderingData() = 0;
    virtual std::shared_ptr<Texture> CreateTexture(uint32 width, uint32 height) const = 0;
    virtual std::shared_ptr<RenderTarget> CreateRenderTarget(uint32 width, uint32 height) = 0;

    virtual void OnWindowDestroyed(Window* window) = 0;

    virtual uint32 GetBufferCount() const = 0;

    EventQueue<RenderingSubsystem>& GetEventQueue();

private:
    EventQueue<RenderingSubsystem> _eventQueue;
    std::unique_ptr<ShaderChangeListener> _shaderChangeListener;
};
