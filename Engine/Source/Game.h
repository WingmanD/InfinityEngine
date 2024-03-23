#pragma once

#include "Asset.h"
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
};
