#include "ShaderChangeListener.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Rendering/Shader.h"

ShaderChangeListener::ShaderChangeListener(const std::filesystem::path& directory) : Platform::DirectoryChangeListener(directory)
{
}

void ShaderChangeListener::OnFileCreated(const std::filesystem::path& path) const
{
}

void ShaderChangeListener::OnFileModified(const std::filesystem::path& path) const
{
    const AssetManager& assetManager = AssetManager::Get();

    std::shared_ptr<Shader> shader = assetManager.FindAssetByName<Shader>(Name(path.stem()));
    if (shader == nullptr)
    {
        return;
    }

    shader->Recompile();
}

void ShaderChangeListener::OnFileDeleted(const std::filesystem::path& path) const
{
}
