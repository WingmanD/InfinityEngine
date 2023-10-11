#pragma once

#include <memory>
#include "Core.h"
#include "EventQueue.h"
#include "Engine/Subsystems/EngineSubsystem.h"

class StaticMesh;
class Window;
class Texture;
class RenderTarget;

class RenderingSubsystem : public EngineSubsystem
{
public:
    RenderingSubsystem();

    virtual std::shared_ptr<Window> ConstructWindow(const std::wstring& title) = 0;

    virtual std::shared_ptr<StaticMesh> CreateStaticMesh(const std::string& name) = 0;
    virtual std::shared_ptr<Texture> CreateTexture(uint32 width, uint32 height) const = 0;
    virtual std::shared_ptr<RenderTarget> CreateRenderTarget(uint32 width, uint32 height) = 0;

    virtual void OnWindowDestroyed(Window* window) = 0;

    virtual uint32 GetBufferCount() const = 0;

    EventQueue<RenderingSubsystem>& GetEventQueue();

private:
    EventQueue<RenderingSubsystem> _eventQueue;
};
