#pragma once

#include "Containers/EventQueue.h"
#include "Engine/Subsystems/EngineSubsystem.h"
#include "Filesystem/ShaderChangeListener.h"
#include "Rendering/InstanceBuffer.h"
#include "Rendering/Widgets/WidgetRenderingProxy.h"
#include <memory>

#include "Containers/GPUBuffer.h"

class GPUBuffer;
class World;
class Window;
class StaticMeshRenderingData;
class StaticMeshRenderingSystem;
class MaterialRenderingData;
class MaterialParameterRenderingData;
class MaterialParameterMap;
class Texture;
class RenderTarget;
struct MaterialParameter;
struct Scene;

class RenderingSubsystem : public EngineSubsystem
{
public:
    RenderingSubsystem();

    static RenderingSubsystem& Get();


    virtual std::shared_ptr<Window> ConstructWindow(const std::wstring& title) = 0;
    virtual void ForEachWindow(std::function<bool(Window*)> callback) = 0;

    virtual std::unique_ptr<StaticMeshRenderingData> CreateStaticMeshRenderingData() = 0;
    virtual std::unique_ptr<MaterialRenderingData> CreateMaterialRenderingData() = 0;
    virtual std::unique_ptr<MaterialParameterRenderingData> CreateMaterialParameterRenderingData() = 0;
    virtual std::shared_ptr<Texture> CreateTexture(uint32 width, uint32 height) const = 0;
    virtual std::shared_ptr<RenderTarget> CreateRenderTarget(uint32 width, uint32 height) = 0;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateDefaultWidgetRenderingProxy() = 0;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateTextWidgetRenderingProxy() = 0;
    virtual std::unique_ptr<WidgetRenderingProxy> CreateViewportWidgetRenderingProxy() = 0;

    virtual void RegisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system) = 0;
    virtual void UnregisterStaticMeshRenderingSystem(StaticMeshRenderingSystem* system) = 0;

    virtual void InitializeMaterialInstanceBuffer(DynamicGPUBuffer<MaterialParameter>& instanceBuffer, Type* type) = 0;

    virtual std::unique_ptr<GPUBuffer> CreateBuffer(GPUBuffer::EType flags) = 0;

    virtual void OnWindowDestroyed(Window* window) = 0;

    virtual uint32 GetBufferCount() const = 0;

    EventQueue<RenderingSubsystem>& GetEventQueue();

    std::shared_ptr<Scene> GetScene() const;

protected:
    virtual bool Initialize() override;
    virtual bool PostInitialize() override;
    virtual void Tick(double deltaTime) override;

private:
    EventQueue<RenderingSubsystem> _eventQueue;
    std::unique_ptr<ShaderChangeListener> _shaderChangeListener;
    std::shared_ptr<Scene> _scene = nullptr;
};
