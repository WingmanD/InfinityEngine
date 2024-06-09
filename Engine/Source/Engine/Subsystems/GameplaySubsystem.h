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

    bool StartGame(const SharedObjectPtr<ViewportWidget>& viewport, const SharedObjectPtr<Game>& game = nullptr);
    void StopGame();

    SharedObjectPtr<Game> GetGame() const;

    template <typename T> requires IsA<T, Game>
    SharedObjectPtr<T> GetGame() const
    {
        return std::dynamic_pointer_cast<T>(GetGame());
    }

    // todo temporary
    BucketArray<World>& GetWorlds();

    // todo temporary
    SharedObjectPtr<ViewportWidget> GetMainViewport() const;

    // EngineSubsystem
protected:
    virtual bool Initialize() override;
    virtual void Tick(double deltaTime) override;
    virtual void Shutdown() override;

private:
    BucketArray<World> _worlds;
    SharedObjectPtr<Game> _game;

    // todo temporary until we implement players
    std::weak_ptr<ViewportWidget> _viewport;
};
