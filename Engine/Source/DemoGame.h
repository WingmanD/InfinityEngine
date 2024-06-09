#pragma once

#include "Game.h"
#include "Level.h"
#include "DemoGame.reflection.h"

REFLECTED()
class DemoGame : public Game
{
    GENERATED()
    
    // Game
protected:
    virtual void OnInitializeWorld(World& world) override;

private:
    PROPERTY(Edit, Serialize, DisplayName = "Level")
    AssetPtr<Level> _level;
};
