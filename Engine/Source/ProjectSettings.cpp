#include "ProjectSettings.h"
#include "LevelEditorGame.h"
#include "Engine/Subsystems/AssetManager.h"

SharedObjectPtr<ProjectSettings> ProjectSettings::Get()
{
    static std::weak_ptr instance = AssetManager::Get().FindOrCreateAssetByName<ProjectSettings>(Name(L"ProjectSettings"));
    return instance.lock();
}

Type* ProjectSettings::GetGameType() const
{
    return _gameType;
}

SharedObjectPtr<Game> ProjectSettings::GetGame() const
{
    return _game;
}

SharedObjectPtr<Game> ProjectSettings::GetLevelEditorGame() const
{
    return _levelEditorGame;
}
