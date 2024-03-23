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
    static std::shared_ptr<ProjectSettings> Get();

    Type* GetGameType() const;
    
private:
    PROPERTY(Edit, Serialize, DisplayName = "Game Type")
    SubtypeOf<Game> _gameType = Game::StaticType();
};
