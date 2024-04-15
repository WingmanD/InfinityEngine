#include "ProjectSettings.h"
#include "Engine/Subsystems/AssetManager.h"

std::shared_ptr<ProjectSettings> ProjectSettings::Get()
{
    static std::weak_ptr instance = AssetManager::Get().FindOrCreateAssetByName<ProjectSettings>(Name(L"ProjectSettings"));
    return instance.lock();
}

Type* ProjectSettings::GetGameType() const
{
    return _gameType;
}

std::shared_ptr<Game> ProjectSettings::GetGame() const
{
    return _game;
}
