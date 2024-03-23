#pragma once

#include "EngineSubsystem.h"
#include "ECS/World.h"

class Game;

class GameplaySubsystem : public EngineSubsystem
{
public:
    GameplaySubsystem() = default;

    bool StartGame();
    void StopGame();

    std::shared_ptr<Game> GetGame() const;

    // EngineSubsystem
protected:
    virtual bool Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;

private:
    BucketArray<World> _worlds;
    std::shared_ptr<Game> _game;
};
