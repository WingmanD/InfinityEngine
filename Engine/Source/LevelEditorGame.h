#pragma once

#include "Game.h"
#include "LevelEditorGame.reflection.h"

class ViewportWidget;
class Level;

REFLECTED()
class LevelEditorGame : public Game
{
    GENERATED()

public:
    LevelEditorGame();
    
    void SetLevel(const std::shared_ptr<Level>& level);
    std::shared_ptr<Level> GetLevel() const;
    
    // Game
public:
    virtual void OnInitializeWorld(World& world) override;

private:
    std::weak_ptr<ViewportWidget> _viewport;
    std::shared_ptr<Level> _level;
};
