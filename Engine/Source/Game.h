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
    void Shutdown(PassKey<GameplaySubsystem>);

    bool IsRunning() const;

protected:
    virtual bool OnStartup();
    virtual void OnShutdown();

private:
    bool _isRunning = false;

    PROPERTY(Edit, Serialize, DisplayName = "Player Template")
    AssetPtr<EntityTemplate> _playerTemplate;
    
    PROPERTY(Edit, Serialize, DisplayName = "Enemy Template")
    AssetPtr<EntityTemplate> _enemyTemplate;
};
