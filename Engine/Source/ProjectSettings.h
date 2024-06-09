#pragma once

#include "Asset.h"
#include "Game.h"
#include "SubtypeOf.h"
#include "ProjectSettings.reflection.h"

REFLECTED()
class ProjectSettings : public Asset
{
    GENERATED()
    
public:
    static SharedObjectPtr<ProjectSettings> Get();

    Type* GetGameType() const;
    SharedObjectPtr<Game> GetGame() const;
    SharedObjectPtr<Game> GetLevelEditorGame() const;
    
private:
    PROPERTY(Edit, Serialize, DisplayName = "Game Type")
    SubtypeOf<Game> _gameType = Game::StaticType();

    PROPERTY(Edit, Serialize, DisplayName = "Game")
    AssetPtr<Game> _game;

    PROPERTY(Edit, Serialize, DisplayName = "Level Editor Game")
    AssetPtr<Game> _levelEditorGame;
};
