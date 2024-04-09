#pragma once

#include "EngineSubsystem.h"
#include "ECS/World.h"

class ViewportWidget;
class Game;

class GameplaySubsystem : public EngineSubsystem
{
public:
    static GameplaySubsystem& Get();
    
    GameplaySubsystem() = default;

    bool StartGame(const std::shared_ptr<ViewportWidget>& viewport);
    void StopGame();

    std::shared_ptr<Game> GetGame() const;

    // todo temporary
    BucketArray<World>& GetWorlds();

    // todo temporary
    std::shared_ptr<ViewportWidget> GetMainViewport() const;

    // EngineSubsystem
protected:
    virtual bool Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;

private:
    BucketArray<World> _worlds;
    std::shared_ptr<Game> _game;

    // todo temporary until we implement players
    std::weak_ptr<ViewportWidget> _viewport;
};
