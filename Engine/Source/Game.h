#pragma once

#include "Asset.h"
#include "AssetPtr.h"
#include "ECS/EntityTemplate.h"
#include "Game.reflection.h"

class GameplaySubsystem;

REFLECTED()
class Game : public Asset
{
    GENERATED()
    
public:
    bool Startup(PassKey<GameplaySubsystem>);
    void InitializeWorld(World& world, PassKey<GameplaySubsystem>);
    void Shutdown(PassKey<GameplaySubsystem>);

    bool IsRunning() const;

    std::shared_ptr<EntityTemplate> GetPlayerTemplate() const;

protected:
    virtual void OnInitializeWorld(World& world);
    virtual bool OnStartup();
    virtual void OnShutdown();

    void SetShouldAutoFocusViewportOnStartup(bool value);
    bool ShouldAutoFocusViewportOnStartup() const;

private:
    bool _isRunning = false;
    bool _autoFocusViewport = true;

    PROPERTY(Edit, Serialize, DisplayName = "Player Template")
    AssetPtr<EntityTemplate> _playerTemplate;
};
