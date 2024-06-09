#pragma once

#include "Game.h"
#include "Level.h"
#include "LevelEditorGame.reflection.h"

class ViewportWidget;
class Level;

REFLECTED()
class LevelEditorGame : public Game
{
    GENERATED()

public:
    LevelEditorGame();
    
    void SetLevel(const SharedObjectPtr<Level>& level);
    SharedObjectPtr<Level> GetLevel() const;
    
    // Game
public:
    virtual void OnInitializeWorld(World& world) override;

private:
    std::weak_ptr<ViewportWidget> _viewport;
    SharedObjectPtr<Level> _level;
};
