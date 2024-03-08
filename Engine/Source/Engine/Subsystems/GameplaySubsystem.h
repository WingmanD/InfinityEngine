#pragma once

#include "EngineSubsystem.h"
#include "ECS/World.h"

class GameplaySubsystem : public EngineSubsystem
{
public:
    GameplaySubsystem() = default;

    // EngineSubsystem
protected:
    virtual bool Initialize() override;
    virtual void Tick(double deltaTime) override;

private:
    BucketArray<World> _worlds;
};
